#include "hxl-lang/services/parser.h"
#include <iostream>

HXL::Result<HXL::Document> HXL::Parser::parse(const std::vector<Token> &tokens) {
    if (tokens.empty()) {
        return Error{.errorCode = ErrorCode::HXL_EMPTY, .message = "Source is empty."};
    } else if (tokens[tokens.size() - 1].tokenType != T::T_NEWLINE) {
        return Error{.errorCode = ErrorCode::HXL_INVALID_EOF, .message = "Source must end with an empty line."};
    }

    std::vector<Node> nodes;

    // The index of the node we're currently working on
    // ``std::nullopt``, when not working on any nodes
    std::optional<size_t> currentNode;

    // As we traverse the list of tokens, this enum helps us understand
    // what we have "just seen" -- the context -- in which we're working
    enum class GrammaticalContext {
        StartOfLine,

        PropertyKey,
        PropertyValue,
        NodeType,
        Inheritance,

        AfterNodeType,
        AfterNodeName,

        ExpandingArray_ExpectsValue,
        ExpandingArray_GotValue,
        EndedArrayExpansion,
    } context = GrammaticalContext::StartOfLine;

    // Short-hand for GrammaticalContext
    typedef GrammaticalContext GC;

    // Describes the entire line, for instance of it's detected
    // as a line which declares a node, or one which declares a property.
    enum class Sentence {
        NotDetermined,
        Node,
        NodeProperty,
    } sentence = Sentence::NotDetermined;

    // When a delimiter indicating a special property (reference or array)
    enum class PropertySpecialization {
        None,
        Reference,
        Array,
    };

    // A temporary version of a node property, getting assembled as a
    // node property expression is traversed
    struct BuildingProperty {
        std::string key;
        PropertySpecialization specialization = PropertySpecialization::None;
        std::vector<std::string> values;
        DataType dataType;

        /**
         * Short-hand for adding values
         *
         * @param value
         */
        void operator+=(const Token &token) {
            if (!token.value.has_value()) {
                return;
            }

            values.push_back(token.toString());

            if (specialization == PropertySpecialization::Reference) {
                dataType = DataType::NodeRef;
                return;
            }

            switch (token.tokenType) {
                case TokenType::T_STRING_LITERAL:
                    dataType = DataType::String;
                    break;
                case TokenType::T_BOOL:
                    dataType = DataType::Bool;
                    break;
                case TokenType::T_INT:
                    dataType = DataType::Int;
                    break;
                case TokenType::T_FLOAT:
                    dataType = DataType::Float;
                    break;
                default:
                    assert(false && "Data type mapping in parser has failed.");
            }
        }
    };

    std::optional<BuildingProperty> buildingProperty;

    // In this loop, the idea is to first of all look at the current token
    // Every token type has different behavior and expectations.
    // For each token type, we check the contexts that it fits into
    // Ultimately, if no criteria is met, it will be considered an
    // "Unexpected token."
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        const Token &token = *it;
        auto peekIt = std::next(it);
        std::optional<Token> peek;
        if (peekIt != tokens.end()) {
            peek = *peekIt;
        }

        switch (token.tokenType) {
            /**
             * Delimiters
             */
            case TokenType::T_DELIMITER: {
                if (!token.value.has_value()) {
                    return unexpectedTokenError(token);
                }
                std::string tk = token.value.value();

                if (context == GC::AfterNodeName && tk == "<=") {
                    context = GC::Inheritance;
                } else if (context == GC::ExpandingArray_GotValue && tk == ",") {
                    context = GC::ExpandingArray_ExpectsValue;
                } else if (sentence == Sentence::NotDetermined && tk == "<") {
                    context = GC::NodeType;
                    sentence = Sentence::Node;
                } else if (context == GC::NodeType && tk == ">") {
                    context = GC::AfterNodeType;
                } else if (context == GC::PropertyKey && tk == ":") {
                    context = GC::PropertyValue;
                    if (peek.has_value() && peek->tokenType != TokenType::T_WHITESPACE) {
                        return unexpectedTokenError(peek.value());
                    }
                } else if (context == GC::PropertyKey && tk == "[]") {
                    buildingProperty->specialization = PropertySpecialization::Array;
                } else if (context == GC::PropertyKey && tk == "&") {
                    buildingProperty->specialization = PropertySpecialization::Reference;
                } else {
                    return unexpectedTokenError(token);
                }

                break;
            }

            /**
             * Punctuators
             */
            case TokenType::T_PUNCTUATOR: {
                if (!token.value.has_value()) {
                    return unexpectedTokenError(token);
                }

                std::string tk = token.value.value();
                if (context == GC::PropertyValue && tk != "{" &&
                    buildingProperty->specialization != PropertySpecialization::Array) {
                    return Error{
                            .errorCode = ErrorCode::HXL_ILLEGAL_DATA_TYPE,
                            .message = std::format("Property {} is not declared as array.", tk),
                    };
                }
                if (context == GC::PropertyValue && tk == "{") {
                    context = GC::ExpandingArray_ExpectsValue;
                } else if (context == GC::ExpandingArray_GotValue && tk == "}") {
                    context = GC::EndedArrayExpansion;
                } else {
                    return unexpectedTokenError(token);
                }
                break;
            }

                /**
                 * Identifiers
                 */
            case TokenType::T_IDENTIFIER: {
                if (!token.value.has_value()) {
                    return unexpectedTokenError(token);
                }

                std::string tk = token.value.value();

                if (context == GC::NodeType) {
                    nodes.push_back({tk});
                    currentNode = nodes.size() - 1;
                } else if (context == GC::Inheritance) {
                    nodes[currentNode.value()].inheritance = {.from = tk};
                } else if (context == GC::AfterNodeType) {
                    nodes[currentNode.value()].name = tk;
                    context = GC::AfterNodeName;
                } else if (context == GC::PropertyKey) {
                    buildingProperty = BuildingProperty{
                            .key = tk,
                    };
                } else if (context == GC::PropertyValue && buildingProperty.has_value() &&
                           buildingProperty->specialization == PropertySpecialization::Reference) {
                    buildingProperty.value() += token;
                } else {
                    return unexpectedTokenError(token);
                }
                break;
            }

                /**
                 * Ordinary whitespace
                 */
            case TokenType::T_WHITESPACE:
                // NODE.005
                if (context == GC::PropertyKey) {
                    return unexpectedTokenError(token);
                }
                break;

                /**
                 * New-line
                 *
                 * When a new-line is countered, we "reset" the contextual understanding
                 */
            case TokenType::T_NEWLINE:
                // If we're building a node property, we append it to the
                // syntax tree.
                if (currentNode.has_value() && sentence == Sentence::NodeProperty) {
                    nodes[currentNode.value()].properties.push_back({
                                                                            .name = buildingProperty->key,
                                                                            .values = buildingProperty->values,
                                                                            .dataType = buildingProperty->dataType,
                                                                    });
                }

                // Reset the context, as we enter a new line
                context = GC::StartOfLine;
                sentence = Sentence::NotDetermined;
                break;

                /**
                 * TAB
                 *
                 * Tab (at beginning of line) indicates that we're
                 * building a node property.
                 */
            case TokenType::T_TAB:
                switch (context) {
                    case GC::StartOfLine:
                        if (!currentNode.has_value()) {
                            return unexpectedTokenError(token);
                        }
                        context = GC::PropertyKey;
                        sentence = Sentence::NodeProperty;
                        break;
                    default:
                        return unexpectedTokenError(token);
                }
                break;

                /**
                 * Data types
                 *
                 * All data types should be added to the temporary "Building property"
                 * object. If that object isn't actively used, then a data type token
                 * was used somewhere it shouldn't.
                 */
            case TokenType::T_STRING_LITERAL:
            case TokenType::T_INT:
            case TokenType::T_FLOAT:
            case TokenType::T_BOOL:
                if (context == GC::PropertyValue || context == GC::ExpandingArray_ExpectsValue) {
                    buildingProperty.value() += token;
                    context = GC::ExpandingArray_GotValue;
                } else {
                    return unexpectedTokenError(token);
                }
                break;

                /**
                 * If no case fits
                 */
            default:
                return unexpectedTokenError(token);
        }
    }

    return Document{
            .nodes = nodes,
    };
}
