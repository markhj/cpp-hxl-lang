#include "hxl-lang/services/tokenizer.h"
#include <iostream>

uint8_t HXL::Tokenizer::indentSize = 4;

HXL::TokenizerResult HXL::Tokenizer::tokenize(const std::string &source) {
    // Shorthand for readability
    typedef BufferLooksLike BLL;

    std::vector<Token> tokens;
    std::string buffer;
    char peek, prev;

    enum class Context {
        None,
        StringLiteral,
        Indentation,
        Comment,
    } context = Context::Indentation;

    BufferLooksLike bufferLooksLike = BufferLooksLike::Empty;

    // Allocate memory in advance, which is a reasonable expectation
    // for most HXL sources. Avoid unnecessary re-allocations early in the parsing.
    tokens.reserve(200);
    buffer.reserve(50);

    // In a few spots we fast-forward the iterator, i, therefore we cannot
    // safely increment it in the same fashion as line number is incremented,
    // due to the risk that future additions of ++i will result in forgetting
    // to also increment the column. With this solution that isn't an issue.
    uint16_t colOffset = 0;

    // Initial cursor position in source
    SourcePosition pos{1, 1};

    // Used to indicate when the rest of a line should be ignored
    // Used to strip comments from the tokenization process
    bool ignoreRemainderOfLine = false;

    for (int i = 0; i < source.length(); ++i) {
        const char c = source[i];

        pos.col = i - colOffset;
        peek = i < source.length() - 1 && source[i + 1] != '\n' ? source[i + 1] : '\0';
        prev = i > 0 ? source[i - 1] : '\0';

        if (ignoreRemainderOfLine) {
            if (c == '\n') {
                ignoreRemainderOfLine = false;
            } else {
                buffer += c;
                continue;
            }
        }

        // When the cursor is between two quotation marks, we continue
        // to add to the buffer, until we meet the second quotation
        if (context == Context::StringLiteral) {
            if (c == '"') {
                context = Context::None;
                tokens.push_back({T::T_STRING_LITERAL, buffer, pos});
                buffer.clear();
            } else if (c == '\n') {
                return Error{
                        ErrorCode::HXL_ILLEGAL_WHITESPACE,
                        std::format("[Line {}, Col {}] Illegal whitespace", pos.line, pos.col)};
            } else {
                buffer += c;
            }

            continue;
        }

        // CMT.004
        // Check that single-line comment starts at the beginning of the line
        if (context == Context::Indentation && peek == '#') {
            return Error{
                    ErrorCode::HXL_ILLEGAL_WHITESPACE,
                    std::format("[Line {}, Col {}] Illegal whitespace", pos.line, pos.col)};
        }

        // If we are parsing an indentation (whitespaces at the beginning of a line)
        // and encounter something that isn't another whitespace character, we set the
        // context back to "None"
        if (context == Context::Indentation && c != ' ' && c != '\r') {
            context = Context::None;
        }

        try {
            switch (c) {
                // Comments
                case '#':
                    // CMT.002
                    // There must be whitespace before the #, unless it's the beginning
                    // of the line
                    if (i > 0 && prev != ' ') {
                        return Error{
                                ErrorCode::HXL_ILLEGAL_WHITESPACE,
                                std::format("[Line {}, Col {}] Illegal whitespace", pos.line, pos.col)};
                    }

                    // CMT.002 (End of line), CMT.004 (Stand-alone)
                    // There must be whitespace after #
                    if (peek != ' ') {
                        return Error{
                                ErrorCode::HXL_ILLEGAL_WHITESPACE,
                                std::format("[Line {}, Col {}] Illegal whitespace", pos.line, pos.col)};
                    }

                    ignoreRemainderOfLine = true;
                    context = Context::Comment;
                    break;

                // Delimiters
                case '<':
                    handleBuffer(buffer, tokens, bufferLooksLike, pos);
                    if (peek == '=') {
                        tokens.push_back({T::T_DELIMITER, "<=", pos});
                        ++i;
                    } else {
                        tokens.push_back({T::T_DELIMITER, charToStr(c), pos});
                    }
                    break;
                case '>':
                case ':':
                case ',':
                case '&':
                    handleBuffer(buffer, tokens, bufferLooksLike, pos);
                    tokens.push_back({T::T_DELIMITER, charToStr(c), pos});
                    break;
                case '[':
                    handleBuffer(buffer, tokens, bufferLooksLike, pos);
                    if (peek == ']') {
                        tokens.push_back({T::T_DELIMITER, "[]", pos});
                        ++i;
                    } else {
                        tokens.push_back({T::T_DELIMITER, charToStr(c), pos});
                    }
                    break;

                    // String literal
                case '"':
                    context = Context::StringLiteral;
                    break;

                    // Punctuators
                case '{':
                case '}':
                    tokens.push_back({T::T_PUNCTUATOR, charToStr(c), pos});
                    break;

                    // Whitespace
                case '\t':
                    tokens.push_back({T::T_TAB, std::nullopt, pos});
                    break;
                case '\n':
                    if (context == Context::Comment) {
                        // CMT.004
                        // Comments cannot be empty
                        if (buffer.empty() || buffer == " ") {
                            return Error{
                                    ErrorCode::HXL_ILLEGAL_COMMENT,
                                    std::format("[Line {}] Illegal comment", pos.line)};
                        }

                        // Empty whatever has been collected in comment context
                        buffer.clear();
                    }

                    handleBuffer(buffer, tokens, bufferLooksLike, pos);
                    tokens.push_back({T::T_NEWLINE, std::nullopt, pos});
                    ++pos.line;
                    colOffset = i - 1;
                    context = Context::Indentation;
                    break;
                case ' ':
                    if (context == Context::Indentation) {
                        if (pos.col == 5) {
                            tokens.push_back({T::T_TAB, std::nullopt, pos});
                            context = Context::None;
                        }
                    } else if (peek == '#') {
                        handleBuffer(buffer, tokens, bufferLooksLike, pos);
                    } else {
                        handleBuffer(buffer, tokens, bufferLooksLike, pos);
                        tokens.push_back({T::T_WHITESPACE, std::nullopt, pos});
                    }
                    break;

                    // GEN.003 states that \r must be ignored.
                case '\r':
                    // Ignore
                    break;

                default:
                    bool isDigit = isdigit(c);
                    bool isAlpha = isalpha(c);

                    if ((isDigit || c == '-') && bufferLooksLike == BLL::Empty) {
                        bufferLooksLike = BLL::Integer;
                    } else if (c == '.' && bufferLooksLike == BLL::Integer) {
                        bufferLooksLike = BLL::Float;
                    } else if (isAlpha || (c == '_' && bufferLooksLike == BLL::Identifier)) {
                        bufferLooksLike = BLL::Identifier;
                    } else if (isDigit && (bufferLooksLike == BLL::Integer || bufferLooksLike == BLL::Float ||
                                           bufferLooksLike == BLL::Identifier)) {
                        // Do nothing
                    } else {
                        return Error{
                                ErrorCode::HXL_SYNTAX_ERROR,
                                std::format("[Line {}] Unexpected token: {}", pos.line, c)};
                    }

                    buffer += c;
            }
        } catch (SyntaxError &e) {
            return Error{ErrorCode::HXL_SYNTAX_ERROR,
                         std::format("[Line {}] {}", pos.line, e.what())};
        }
    }

    return tokens;
}

std::string HXL::Tokenizer::charToStr(char c) {
    return std::string() + c;
}

void HXL::Tokenizer::handleBuffer(std::string &buffer,
                                  std::vector<Token> &tokenList,
                                  BufferLooksLike &bufferLooksLike,
                                  const SourcePosition &pos) {
    if (buffer.empty()) {
        return;
    }

    if (buffer == "true" || buffer == "false") {
        tokenList.push_back({T::T_BOOL, buffer, pos});
    } else if (bufferLooksLike == BufferLooksLike::Integer) {
        tokenList.push_back({T::T_INT, buffer, pos});
    } else if (bufferLooksLike == BufferLooksLike::Float) {
        tokenList.push_back({T::T_FLOAT, buffer, pos});
    } else if (bufferLooksLike == BufferLooksLike::Identifier) {
        tokenList.push_back({T::T_IDENTIFIER, buffer, pos});
    } else {
        throw SyntaxError(std::format("Syntax error in: {}", buffer));
    }

    bufferLooksLike = BufferLooksLike::Empty;
    buffer.clear();
}
