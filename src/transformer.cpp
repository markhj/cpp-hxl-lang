#include "hxl-lang/services/transformer.h"


void HXL::Transformer::transform(const std::shared_ptr<Document> &document) {
    inheritanceResolution(document);
}

void HXL::Transformer::inheritanceResolution(const std::shared_ptr<Document> &document) {
    std::unordered_map<std::string, Node> nodeMap;

    // Avoid re-allocation
    nodeMap.reserve(document->nodes.size());

    for (Node &node: document->nodes) {
        nodeMap[node.name] = node;

        // If no inheritance is needed
        if (!node.inheritance.has_value()) {
            continue;
        }

        // The schema ensures the node must exist before it is used
        // for inheritance, so we're guaranteed that the parent node
        // exists in the node map.
        // We're furthermore sure that the parent node exists, and
        // don't need to waste computation on checking that.
        Node parent = nodeMap[node.inheritance->from];
        for (const NodeProperty &parentProperty: parent.properties) {
            auto it = std::find_if(node.properties.begin(),
                                   node.properties.end(),
                                   [&](const NodeProperty &item) -> bool {
                                       return item.name == parentProperty.name;
                                   });
            if (it == node.properties.end()) {
                node.properties.push_back(parentProperty);
            }
        }
    }
}
