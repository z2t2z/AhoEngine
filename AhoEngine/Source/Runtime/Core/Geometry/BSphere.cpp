#include "Ahopch.h"
#include "BSphere.h"
#include "Mesh.h"
#include <glm/gtx/norm.hpp>

namespace Aho {
    //center:  Average center of all vertices
    //radiues: Maximal distance between center and vertices
    BSphere BSphere::FromVertices(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        BSphere sphere;
        glm::vec3 c(0.0f);

        for (uint32_t idx : indices) {
            c += vertices[idx].position;
        }
        c /= static_cast<float>(indices.size());

        float maxDistSq = 0.0f;
        for (uint32_t idx : indices) {
            float distSq = glm::length2(vertices[idx].position - c);
            maxDistSq = std::max(maxDistSq, distSq);
        }

        sphere.center = c;
        sphere.radius = std::sqrt(maxDistSq);
        return sphere;
    }
    
	BSphere BSphere::FromMesh(const Mesh& mesh) {
        return FromVertices(mesh.vertexBuffer, mesh.indexBuffer);
	}
}
