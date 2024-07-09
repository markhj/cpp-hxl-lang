#pragma once

#include "hxl-lang/core.h"

#include <memory>

namespace HXL {
    /**
     * The Deserializer is the last stage of translating a HXL source into
     * C++ structures. The deserializer will, based on a protocol that you
     * define, convert every node into structs, classes, etc.
     */
    class Deserializer {
    public:
        /**
         * Perform deserialization on the document based on the provided protocol.
         *
         * @param protocol
         * @param document
         */
        static ErrorList deserialize(const DeserializationProtocol &protocol,
                                const std::shared_ptr<Document> &document);

    private:
        static DeserializedNode generateNode(const Node &node);

        inline static DeserializedValue toValue(const NodeProperty &nodeProperty);
    };
}
