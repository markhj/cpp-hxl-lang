#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace HXL {
    /**
     * Type used to track performance time (in microseconds).
     */
    typedef long long PerformanceTime;

    /**
     * List of error codes.
     *
     * @see https://github.com/markhj/hxl-lang/blob/master/specs/hxl-2024/07-error-codes.md
     *      List of codes in the language spec.
     */
    enum class ErrorCode {
        HXL_EMPTY = 100,
        HXL_INVALID_EOF = 101,
        HXL_UNEXPECTED_TOKEN = 105,
        HXL_UNEXPECTED_TERMINATION = 106,
        HXL_SYNTAX_ERROR = 107,
        HXL_ILLEGAL_WHITESPACE = 110,
        HXL_INVALID_NODE_FORM = 130,
        HXL_INVALID_PROPERTY_FORM = 131,
        HXL_EMPTY_PROPERTY_VALUE = 132,
        HXL_ORPHAN_PROPERTY = 133,
        HXL_ILLEGAL_COMMENT = 140,
        HXL_ARRAY_MIXED_TYPES = 200,
        HXL_ARRAY_UNKNOWN_TYPE = 201,
        HXL_NODE_REFERENCE_NOT_FOUND = 230,
        HXL_CIRCULAR_NODE_REFERENCE = 231,
        HXL_INHERIT_DIFF_TYPES = 250,
        HXL_ILLEGAL_INHERITANCE = 251,
        HXL_ILLEGAL_REFERENCE = 252,
        HXL_INVALID_NODE_TYPE = 300,
        HXL_INVALID_NODE_NAME = 301,
        HXL_INVALID_PROPERTY_KEY = 302,
        HXL_ILLEGAL_FLOAT = 400,
        HXL_ILLEGAL_STRING = 420,
        HXL_NON_UNIQUE_NODE = 500,
        HXL_NON_UNIQUE_PROPERTY = 510,
        HXL_UNKNOWN_NODE_TYPE = 800,
        HXL_ILLEGAL_DATA_TYPE = 830,
        HXL_REQUIRED_PROPERTY_NOT_FOUND = 900,
        HXL_UNKNOWN_PROPERTY = 910,
        HXL_CANNOT_DESERIALIZE_NODE = 1000,
    };

    /**
     * Token types
     */
    enum class TokenType {
        /**
         * Delimiters such as [], & and :
         */
        T_DELIMITER,

        /**
         * Punctuaters such as { and } for arrays
         */
        T_PUNCTUATOR,

        /**
         * Identifiers, such as node types, node names and property keys
         */
        T_IDENTIFIER,

        /**
         * Ordinary whitespace ``\s``
         */
        T_WHITESPACE,

        /**
         * New-line character (``\n``)
         */
        T_NEWLINE,

        /**
         * Tabulator (``\t``)
         */
        T_TAB,

        /**
         * String literal
         */
        T_STRING_LITERAL,

        /**
         * Integer
         */
        T_INT,

        /**
         * Float
         */
        T_FLOAT,

        /**
         * Boolean
         */
        T_BOOL,
    };

    /**
     * List of data types
     *
     * Note that are arrays are constructed with by specifying one of these
     * types alongside a HXL::ValueStructure.
     */
    enum class DataType {
        Bool,
        Float,
        Int,
        String,
        NodeRef,
    };

    /**
     * Used by tokenizer to indicate where a token was found.
     */
    struct SourcePosition {
        uint16_t line, col;
    };

    /**
     * A token, as it looks after tokenization.
     *
     * @note This is not the same Token type which is defined in schemas.
     */
    struct Token {
        /**
         * Token type, such as ``T_DELIMITER`` or ``T_IDENTIFIER``.
         */
        TokenType tokenType;

        /**
         * (Optional) value, given as a string.
         * It will be converted its "real" data type at a later stage
         * of the parsing.
         */
        std::optional<std::string> value;

        /**
         * Position in the source -- i.e. line and column.
         */
        SourcePosition position;

        /**
         * Return a human-readable text which makes it clearer which
         * token it is.
         *
         * @return
         */
        [[nodiscard]] std::string toString() const {
            if (value.has_value()) {
                return value.value();
            }

            switch (tokenType) {
                case TokenType::T_DELIMITER:
                    return "T_DELIMITER";
                case TokenType::T_PUNCTUATOR:
                    return "T_PUNCTUATOR";
                case TokenType::T_IDENTIFIER:
                    return "T_IDENTIFIER";
                case TokenType::T_WHITESPACE:
                    return "T_WHITESPACE";
                case TokenType::T_NEWLINE:
                    return "T_NEWLINE";
                case TokenType::T_TAB:
                    return "T_TAB";
                case TokenType::T_STRING_LITERAL:
                    return "T_STRING_LITERAL";
                case TokenType::T_INT:
                    return "T_INT";
                case TokenType::T_FLOAT:
                    return "T_FLOAT";
                case TokenType::T_BOOL:
                    return "T_BOOL";
                default:
                    return std::to_string(static_cast<int>(tokenType));
            }
        }
    };

    /**
     * Generalized error type.
     */
    struct Error {
        /**
         * Error code, for instance ``HXL_SYNTAX_ERROR`` or ``HXL_UNEXPECTED_TOKEN``.
         */
        ErrorCode errorCode;

        /**
         * Message
         */
        std::string message;
    };

    /**
     * Short-hand for a list of errors.
     */
    typedef std::vector<Error> ErrorList;

    /**
     * Inspired by Rust's Result object, this ``Result``
     * can be used to return a result from a function, where we
     * don't want to throw an exception, but want to handle
     * it gracefully.
     *
     * @tparam T
     */
    template<typename T>
    class Result : public std::variant<Error, T> {
    public:
        using std::variant<Error, T>::variant;

        /**
         * Returns true, if the ``Result`` contains an error.
         * @return
         */
        [[nodiscard]] bool isErr() const {
            return std::holds_alternative<Error>(*this);
        }

        /**
         * Returns the error.
         *
         * @note If you do not type-check ahead of calling this method,
         *      it will cause an error.
         *
         * @return
         */
        [[nodiscard]] Error error() const {
            return std::get<Error>(*this);
        }

        /**
         * Returns the result.
         *
         * @note If you do not type-check ahead of calling this method,
         *      it will cause an error.
         *
         * @return
         */
        T get() const {
            return std::get<T>(*this);
        }
    };

    /**
     * Value which contains information about inheritance (parent node)
     */
    struct Inheritance {
        std::string from;
    };

    /**
     * Forward-declaration of ``NodeProperty``
     */
    struct NodeProperty;

    /**
     * A node after it has been parsed, and as its passed
     * through the different validation and transformation stages.
     */
    struct Node {
        /**
         * The type as provided in the raw source
         */
        std::string type;

        /**
         * The node name, as provided in the source
         */
        std::string name;

        /**
         * Node properties. As this stage still existing as strings.
         */
        std::vector<NodeProperty> properties;

        /**
         * Information about inheritance.
         */
        std::optional<Inheritance> inheritance;
    };

    /**
     * A "raw" node property (as it looks in the HXL source).
     */
    struct NodeProperty {
        /**
         * Name that was provided.
         */
        std::string name;

        /**
         * The values (given as string).
         * It is not validated in any shape or form, as this point.
         * No matter the desired data type this is always a vector,
         * and in case of "single-value" types such as plain int,
         * bool and string, we simply read out the first value.
         */
        std::vector<std::string> values;

        /**
         * The interpreted data type (not validated). This is derived
         * simply from analysis of the string pattern.
         *
         * In case of arrays, the first value determines what the array
         * will look like. Semantic validation will ensure matching values.
         */
        DataType dataType;
    };

    /**
     * The assembled syntax tree (document), still consisting of string values.
     *
     * The document can be used in a number of stages of transformation and validation,
     * before it's passed  on to the deserialization stage.
     */
    struct Document {
        /**
         * List of nodes contained in the document.
         */
        std::vector<Node> nodes;
    };

    /**
     * Forward-declarations
     */
    struct SchemaNodeType;
    struct SchemaNodeProperty;

    /**
     * The schema, which defines which node types are allowed to exist
     * in a document.
     */
    struct Schema {
        /**
         * List of defined node types.
         */
        std::vector<SchemaNodeType> types;
    };

    /**
     * Specification of what a node type looks like, namely
     * its name and its allowed properties.
     */
    struct SchemaNodeType {
        /**
         * Name of the node type.
         */
        std::string name;

        /**
         * List of supported and expected properties.
         */
        std::vector<SchemaNodeProperty> properties;
    };

    enum class ValueStructure {
        Single,
        Array,
    };

    /**
     * Definition of a specific property of a specific node.
     */
    struct SchemaNodeProperty {
        /**
         * Property name (key)
         */
        std::string name;

        /**
         * Expected data type.
         */
        DataType dataType;

        /**
         *
         */
        ValueStructure structure = ValueStructure::Single;

        /**
         * When set to true, the Schema Validation will throw an error,
         * if the property is not provided on a node, in the document.
         */
        bool required = false;
    };

    /**
     * Forward-declaration
     */
    struct DeserializedNodeProperty;

    /**
     * What a node looks like after it has been deserialized.
     */
    struct DeserializedNode {
        std::string name;
        std::map<std::string, DeserializedNodeProperty> properties;
    };

    /**
     * A (deserialized) reference to another node.
     */
    struct NodeRef {
        std::string references;
    };

    /**
     * List of values that can be deserialized to.
     */
    typedef std::variant<bool,
                         int,
                         float,
                         std::string,
                         NodeRef,
                         std::vector<int>,
                         std::vector<float>,
                         std::vector<std::string>>
            DeserializedValue;

    /**
     * What a single deserialized node property looks like.
     */
    struct DeserializedNodeProperty {
        DeserializedValue value;
    };

    /**
     * A handler for a specific node type.
     */
    struct DeserializationHandle {
        /**
         * The node type, the handle targets
         */
        std::string nodeType;

        /**
         * The handle.
         */
        std::function<void(const DeserializedNode &)> handle;
    };

    /**
     * Definitions on how the Deserializer must handle each property.
     */
    struct DeserializationProtocol {
        /**
         * List of handles.
         */
        std::vector<DeserializationHandle> handles;
    };

    /**
     * Execution time reported by performance measurer
     */
    struct ExecutionTime {
        PerformanceTime ms;
    };

    /**
     * Summary of performance on a per-stage basis
     */
    struct PerformanceResults {
        typedef std::optional<ExecutionTime> StageResult;

        StageResult tokenization,
                parsing,
                semanticAnalysis,
                transformer,
                schemaValidation,
                deserialization;

        [[nodiscard]] ExecutionTime getTotal() const {
            PerformanceTime total = 0;

            std::function<PerformanceTime(StageResult)> extract = [](StageResult time) {
                return time.has_value() ? time.value().ms : 0;
            };

            total += extract(tokenization);
            total += extract(parsing);
            total += extract(semanticAnalysis);
            total += extract(transformer);
            total += extract(schemaValidation);
            total += extract(deserialization);

            return {total};
        }
    };
}
