#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TinyButEpic {
    /**
     * Basic 3D entity
     */
    class Entity3D {};

    /**
     * Material
     */
    class Material {
    public:
        std::string name;
        std::string albedoTexture;
        std::array<float, 2> textureUV;
    };

    /**
     * Abstract class for objects that can be rendered
     */
    class Renderable {
    public:
        std::shared_ptr<Material> material;

        virtual void render() = 0;
    };

    /**
     * 3D surface
     */
    class Surface3D : public Entity3D,
                      public Renderable {
    public:
        void render() override;
    };

    /**
     * 3D mesh
     */
    class Mesh3D : public Entity3D,
                   public Renderable {
    public:
        std::string shape;

        void render() override;
    };

    /**
     * Renderer
     */
    class Renderer {
    public:
        /**
         * Render the list of renderables
         *
         * @param renderables
         */
        void render(const std::vector<std::shared_ptr<Renderable>> &renderables);
    };

    /**
     * Container of materials, making searching for a material
     * a bit more convenient.
     */
    class MaterialsContainer : public std::vector<std::shared_ptr<Material>> {
    public:
        std::optional<std::shared_ptr<Material>> get(const std::string &name);
    };
}
