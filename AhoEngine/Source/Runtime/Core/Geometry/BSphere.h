#pragma once
#include "Vertex.h"
#include <glm/glm.hpp>

namespace Aho {
    struct Mesh;
	struct BSphere {
        glm::vec3 center;
        float radius;

        BSphere() : center(0.0f), radius(0.0f) {}

        static BSphere FromVertices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        static BSphere FromMesh(const Mesh& mesh);
	};
}
