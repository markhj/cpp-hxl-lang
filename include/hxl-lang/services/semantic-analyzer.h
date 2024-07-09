#pragma once

#include "hxl-lang/core.h"

#include <memory>
#include <vector>
#include <format>

namespace HXL {
    /**
     * The Semantic Analyzer is responsible for a number of integrity checks
     * in the document, such as verifying uniqueness of node names.
     *
     * @see https://github.com/markhj/hxl-lang/blob/master/specs/hxl-2024/05-semantic-rules.md
     *      List of semantic rules per the language specification
     */
    class SemanticAnalyzer {
    public:
        /**
         * Analyze a document's semantic structure and return a list of errors.
         *
         * @param document
         * @return
         */
        static ErrorList analyze(const std::shared_ptr<Document> &document);
    };
}
