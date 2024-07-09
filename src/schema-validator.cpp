#include "hxl-lang/services/schema-validator.h"

HXL::ErrorList HXL::SchemaValidator::validate(const HXL::Schema &schema, const std::shared_ptr<Document> &document) {
    ErrorList errors;

    for (const Node &node: document->nodes) {
        // Look for the node type in the schema
        auto it = std::find_if(schema.types.begin(),
                               schema.types.end(),
                               [&](const SchemaNodeType &nodeType) -> bool {
                                   return nodeType.name == node.type;
                               });

        // If the type isn't found, it's an error.
        if (it == schema.types.end()) {
            errors.push_back({
                    .errorCode = ErrorCode::HXL_UNKNOWN_NODE_TYPE,
                    .message = std::format("Node type not declared in schema: {}", node.type),
            });

            // Avoid bad_alloc error in the next phase
            continue;
        }

        SchemaNodeType schemaForNode = *it;

        // Iterate through the node's properties and verify against the schema
        for (const NodeProperty &nodeProperty: node.properties) {
            std::for_each(schemaForNode.properties.begin(),
                          schemaForNode.properties.end(),
                          [&](const SchemaNodeProperty &schemaNodeProperty) {
                              if (schemaNodeProperty.name != nodeProperty.name) {
                                  return;
                              }

                              if (schemaNodeProperty.structure == ValueStructure::Single && nodeProperty.values.size() != 1) {
                                  errors.push_back({
                                          .errorCode = ErrorCode::HXL_ILLEGAL_DATA_TYPE,
                                          .message = std::format("Property not declared as array: {}", nodeProperty.name),
                                  });
                              }
                          });
        }
    }

    return errors;
}
