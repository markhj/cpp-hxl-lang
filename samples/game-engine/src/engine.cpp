#include "engine.h"

void TinyButEpic::Surface3D::render() {
    std::cout << "3D surface drawn ";
    if (material) {
        std::cout << "with material \"" << material->name << "\" [Texture: \"" << material->albedoTexture << "\", ";
        std::cout << " UV: " << material->textureUV[0] << ", " << material->textureUV[1] << "]";
    } else {
        std::cout << "without material";
    }
    std::cout << std::endl;
}

void TinyButEpic::Mesh3D::render() {
    std::cout << "There is a " << shape << std::endl;
}

void TinyButEpic::Renderer::render(const std::vector<std::shared_ptr<Renderable>> &renderables) {
    std::for_each(renderables.begin(),
                  renderables.end(),
                  [&](const std::shared_ptr<Renderable> &renderable) {
                      renderable->render();
                  });
}

std::optional<std::shared_ptr<TinyButEpic::Material>> TinyButEpic::MaterialsContainer::get(const std::string &name) {
    auto it = std::find_if(begin(),
                           end(),
                           [&](const std::shared_ptr<Material> &item) -> bool {
                               return item->name == name;
                           });
    if (it != end()) {
        return *it;
    }
    return std::nullopt;
}
