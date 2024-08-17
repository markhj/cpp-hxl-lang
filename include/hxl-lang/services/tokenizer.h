#pragma once

#include "hxl-lang/core.h"
#include <regex>
#include <vector>
#include <format>

using T = HXL::TokenType;

namespace HXL {
    typedef Result<std::vector<Token>> TokenizerResult;

    /**
     * Syntax error exception
     */
    class SyntaxError : public std::runtime_error  {
    public:
        using std::runtime_error ::runtime_error ;
    };

    class Tokenizer {
    public:
        /**
         * The first stage of translation is tokenizing the source.
         *
         * The source will be broken into tens, hundreds, thousands
         * or millions on small "pieces" (tokens), which make it easier
         * for later stages to work with the document.
         *
         * @param source
         * @return
         */
        static TokenizerResult tokenize(const std::string &source);

    private:
        enum class BufferLooksLike {
            Empty,
            Integer,
            Float,
            Bool,
            String,
            Identifier,
        };

        /**
         * The (exact) size of whitespace indentation that can be
         * converted to tabs, when given at the beginning of a line.
         */
        static uint8_t indentSize;

        inline static void handleBuffer(std::string &buffer,
                                        std::vector<Token> &tokenList,
                                        BufferLooksLike &bufferLooksLike,
                                        const SourcePosition &pos);

        inline static std::string charToStr(char c);
    };
}
