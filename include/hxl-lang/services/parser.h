#pragma once

#include "hxl-lang/traits/traits.h"

#include <cassert>
#include <memory>

using T = HXL::TokenType;

namespace HXL {
    class Parser : protected Traits::HandlesErrors {
    public:
        /**
         * Parse the set of ordered tokens.
         *
         * The result will be a syntax tree, which can be consumed by
         * later validation stages as well as the deserializer.
         *
         * This method also validates a large set of grammatical rules.
         *
         * @param tokens
         * @return
         */
        static Result<Document> parse(const std::vector<Token> &tokens);

    private:
        /**
         * The RuleMismatch is in place to help us locate where
         * unexpected tokens occur. When a rule is tested, and it can
         * no longer match the token block, it will return the position.
         */
        struct Mismatch {
            /**
             * The location in the token block at which the rule could
             * no longer identify more tokens.
             */
            unsigned int at = 0;

            /**
             * The token which could not be recognized by the rule.
             */
            Token token;
        };
    };
}
