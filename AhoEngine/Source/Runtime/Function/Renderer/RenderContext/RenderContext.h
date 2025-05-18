#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Aho {
    class Shader;
    class MeshResource;
    class TextureResource;

    struct Renderable {
        glm::mat4 transform;
        MeshResource* mesh{ nullptr };
        TextureResource* texture{ nullptr };
        Shader* shader{ nullptr };
    };

    struct RenderContext {
        std::vector<Renderable> drawList;
    };
}
