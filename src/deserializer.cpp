#include "hxl-lang/services/deserializer.h"
#include "hxl-lang/utilities/helpers.h"
#include <format>

HXL::ErrorList HXL::Deserializer::deserialize(const HXL::DeserializationProtocol &protocol,
                                              const std::shared_ptr<Document> &document) {
    ErrorList errors;

    // Before we start executing handles, we need to verify that all of them are present.
    // This needs to be done in a separate loop, before processing, otherwise we risk
    // that some processing takes place, before we're sure we can even finish.
    for (const Node &node: document->nodes) {
        auto it = std::find_if(protocol.handles.begin(),
                               protocol.handles.end(),
                               [&](const DeserializationHandle &handle) -> bool {
                                   return handle.nodeType == node.type;
                               });

        if (it == protocol.handles.end()) {
            errors.push_back({ErrorCode::HXL_CANNOT_DESERIALIZE_NODE,
                              std::format("Missing deserializer for: {}", node.type)});
        }
    }

    if (!errors.empty()) {
        return errors;
    }

    // ... And now for the execution of the handles.
    for (const Node &node: document->nodes) {
        std::for_each(protocol.handles.begin(),
                      protocol.handles.end(),
                      [&](const DeserializationHandle &handle) {
                          if (handle.nodeType == node.type) {
                              handle.handle(generateNode(node));
                          }
                      });
    }

    return errors;
}

HXL::DeserializedNode HXL::Deserializer::generateNode(const HXL::Node &node) {
    DeserializedNode result {
            .name = node.name,
    };

    for (const NodeProperty &nodeProperty: node.properties) {
        result.properties[nodeProperty.name].value = toValue(nodeProperty);
    }

    return result;
}

HXL::DeserializedValue HXL::Deserializer::toValue(const HXL::NodeProperty &nodeProperty) {
    if (nodeProperty.values.size() > 1) {
        switch (nodeProperty.dataType) {
            case DataType::Int:
                return Helpers::toArray<int>(nodeProperty.values);
            case DataType::Float:
                return Helpers::toArray<float>(nodeProperty.values);
            case DataType::String:
                return Helpers::toArray<std::string>(nodeProperty.values);
            default:
                throw std::runtime_error("Data type not allowed in arrays.");
        }
    }

    switch (nodeProperty.dataType) {
        case DataType::Bool:
            return nodeProperty.values[0] == "true";
        case DataType::Float:
            return std::stof(nodeProperty.values[0]);
        case DataType::Int:
            return std::stoi(nodeProperty.values[0]);
        case DataType::NodeRef:
            return NodeRef{nodeProperty.values[0]};
        default:
            return nodeProperty.values[0];
    }
}
