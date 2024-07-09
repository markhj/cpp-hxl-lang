#include "the-game.h"

Schema WatchGrassGrowSimulator::HXLSetup::createSchema() {
    Schema schema;

    SchemaNodeType material{"Material"};
    material.properties.emplace_back("albedo_texture", DataType::String, ValueStructure::Single, true);
    material.properties.emplace_back("texture_uv", DataType::Float, ValueStructure::Array, true);
    schema.types.push_back(material);

    SchemaNodeType surface3d{"Surface3D"};
    surface3d.properties.emplace_back("material", DataType::NodeRef, ValueStructure::Single, true);
    schema.types.push_back(surface3d);

    SchemaNodeType mesh3d{"Mesh3D"};
    mesh3d.properties.emplace_back("shape", DataType::String, ValueStructure::Single, true);
    schema.types.push_back(mesh3d);

    SchemaNodeType enemy{"BirdFlock"};
    enemy.properties.emplace_back("type", DataType::String, ValueStructure::Single, true);
    enemy.properties.emplace_back("count", DataType::Int, ValueStructure::Single, true);
    enemy.properties.emplace_back("attacks_player", DataType::Bool, ValueStructure::Single, true);
    schema.types.push_back(enemy);

    return schema;
}

DeserializationProtocol WatchGrassGrowSimulator::HXLSetup::createDeserialization(std::vector<std::shared_ptr<Renderable>> &renderables,
                                                                                 MaterialsContainer &materials,
                                                                                 std::vector<BirdFlock> &birdFlocks) {
    DeserializationProtocol protocol;

    // Material
    DeserializationHandle material{"Material"};
    material.handle = [&](const DeserializedNode &node) {
        auto albedoTexture = node.properties.at("albedo_texture").value;
        auto uv = std::get<std::vector<float>>(node.properties.at("texture_uv").value);
        materials.push_back(std::make_shared<Material>(Material{
                .name = node.name,
                .albedoTexture = std::get<std::string>(albedoTexture),
                .textureUV = {uv[0], uv[1]},
        }));
    };
    protocol.handles.push_back(material);

    // Surface3D
    DeserializationHandle surface3d{"Surface3D"};
    surface3d.handle = [&](const DeserializedNode &node) {
        Surface3D surface = Surface3D{};
        if (node.properties.contains("material")) {
            NodeRef materialName = std::get<NodeRef>(node.properties.at("material").value);
            auto foundMaterial = materials.get(materialName.references);
            if (foundMaterial.has_value()) {
                surface.material = foundMaterial.value();
            }
        }
        renderables.push_back(std::make_shared<Surface3D>(surface));
    };
    protocol.handles.push_back(surface3d);

    // Mesh3D
    DeserializationHandle mesh3d{"Mesh3D"};
    mesh3d.handle = [&](const DeserializedNode &node) {
        Mesh3D surface;
        surface.shape = std::get<std::string>(node.properties.at("shape").value);
        renderables.push_back(std::make_shared<Mesh3D>(surface));
    };
    protocol.handles.push_back(mesh3d);

    // Bird flock
    DeserializationHandle birdFlock{"BirdFlock"};
    birdFlock.handle = [&](const DeserializedNode &node) {
        birdFlocks.push_back({
                .count = std::get<int>(node.properties.at("count").value),
                .type = std::get<std::string>(node.properties.at("type").value),
                .attacksPlayer = std::get<bool>(node.properties.at("attacks_player").value),
        });
    };
    protocol.handles.push_back(birdFlock);

    return protocol;
}
