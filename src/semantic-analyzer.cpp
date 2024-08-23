#include "hxl-lang/services/semantic-analyzer.h"
#include <unordered_set>

HXL::ErrorList HXL::SemanticAnalyzer::analyze(const std::shared_ptr<Document> &document) {
    ErrorList errors;

    // NODE.200: Node name uniqueness
    // NODE.201: Node property uniqueness

    std::unordered_set<std::string> seenNames;
    for (const Node &node: document->nodes) {
        if (seenNames.find(node.name) != seenNames.end()) {
            errors.push_back({
                                     .errorCode = ErrorCode::HXL_NON_UNIQUE_NODE,
                                     .message = std::format("Node name \"{}\" is not unique.", node.name),
                             });
        } else {
            seenNames.insert(node.name);
        }

        if (node.inheritance.has_value()) {
            auto findInhr = seenNames.find(node.inheritance.value().from);
            if (findInhr == seenNames.end()) {
                errors.push_back({
                                         .errorCode = ErrorCode::HXL_ILLEGAL_INHERITANCE,
                                         .message = std::format("Node {} attempts to inherit {} which does not exist.",
                                                                node.name,
                                                                node.inheritance.value().from),
                                 });
            }
        }

        std::vector<std::string> foundPropKeys;
        for (const NodeProperty &nodeProperty: node.properties) {
            // If the property key is already on the list of found ones, then it's
            // definitely not unique
            if (std::find(foundPropKeys.begin(), foundPropKeys.end(), nodeProperty.name) != foundPropKeys.end()) {
                errors.push_back({
                                         .errorCode = ErrorCode::HXL_NON_UNIQUE_PROPERTY,
                                         .message = std::format(R"(Property "{}" under "{}" is not unique.)",
                                                                nodeProperty.name,
                                                                node.name),
                                 });
            }
            foundPropKeys.push_back(nodeProperty.name);

            if (nodeProperty.dataType == DataType::NodeRef) {
                auto referencedNode = nodeProperty.values[0];
                auto findRef = seenNames.find(referencedNode);
                if (node.name == referencedNode) {
                    errors.push_back({
                                             .errorCode = ErrorCode::HXL_ILLEGAL_REFERENCE,
                                             .message = std::format(R"({}:{} is referencing itself.)",
                                                                    node.name,
                                                                    nodeProperty.name),
                                     });
                } else if (findRef == seenNames.end()) {
                    errors.push_back({
                                             .errorCode = ErrorCode::HXL_NODE_REFERENCE_NOT_FOUND,
                                             .message = std::format(
                                                     R"(Referenced node "{}" under {}:{} was not found.)",
                                                     referencedNode,
                                                     node.name,
                                                     nodeProperty.name),
                                     });
                }
            }
        }
    }

    return errors;
}
