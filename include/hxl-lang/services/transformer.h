#pragma once

#include "hxl-lang/core.h"

#include <memory>

namespace HXL {
    /**
     * Implementation of the transformer stage, which handles inheritance
     * resolution (and population of inherited properties).
     */
    class Transformer {
    public:
        /**
         * Perform transformations on the provided document.
         *
         * The document will be scanned for inheritance. Found inheritances
         * will be resolved, and properties to be inherited will be populated.
         *
         * @param document
         */
        static void transform(const std::shared_ptr<Document> &document);

    private:
        /**
         * Handle the inheritance resolution, which is the process of
         * populating a node's inherited properties -- that is, properties
         * which aren't present on the child, but on the parent.
         *
         * Properties which already exist on a child node are not to be modified.
         *
         * @param document
         */
        inline static void inheritanceResolution(const std::shared_ptr<Document> &document);

    };
}
