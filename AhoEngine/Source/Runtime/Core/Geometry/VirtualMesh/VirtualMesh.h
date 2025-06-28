#pragma once

#include "../BoundingSphere.h"
#include <vector>
#include <string>

namespace Aho {
    struct Vertex;
    struct Meshlet {
        std::vector<uint32_t> uniqueVertexIndices;
        std::vector<uint32_t> localIndices;
    };

    class VirtualMeshBuilder {
    public:
        static std::vector<Meshlet> BuildMeshlets(const std::vector<uint32_t>& indexBuffer, size_t maxTriangles = 124, size_t maxVertices = 64);
        static void ExportMeshletsAsOBJ(const std::vector<Meshlet>& meshlets, const std::vector<Vertex>& vertexBuffer, const std::string& filename);
        static std::vector<Meshlet> BuildLODLevel(const std::vector<Meshlet>& previousLevel, size_t groupSize);
    };
}
