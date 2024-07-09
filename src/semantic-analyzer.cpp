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
        }
    }

    return errors;
}
