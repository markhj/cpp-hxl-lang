#pragma once

#include "hxl-lang/core.h"

#include <format>
#include <memory>
#include <vector>

namespace HXL {
    /**
     * Holds a document up against the schema.
     */
    class SchemaValidator {
    public:
        /**
         * Validate a document against a schema.
         *
         * @param schema
         * @param document
         * @return
         */
        static ErrorList validate(const Schema &schema,
                                  const std::shared_ptr<Document> &document);
    };
}
