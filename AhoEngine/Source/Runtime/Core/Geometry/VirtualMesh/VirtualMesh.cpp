#include "Ahopch.h"
#include "VirtualMesh.h"
#include "../Vertex.h"
#include <fstream>
#include <set>

namespace Aho {
	std::vector<Meshlet> VirtualMeshBuilder::BuildMeshlets(const std::vector<uint32_t>& indexBuffer, size_t maxTriangles, size_t maxVertices) {
        std::vector<Meshlet> meshlets;
        std::unordered_map<uint32_t, uint32_t> globalToLocal;

        Meshlet current;

        for (size_t i = 0; i < indexBuffer.size(); i += 3) {
            uint32_t tri[3] = { indexBuffer[i], indexBuffer[i + 1], indexBuffer[i + 2] };
            std::set<uint32_t> newVertices;

            for (int j = 0; j < 3; ++j)
                if (globalToLocal.find(tri[j]) == globalToLocal.end())
                    newVertices.insert(tri[j]);

            if ((current.localIndices.size() / 3 + 1 > maxTriangles) ||
                (current.uniqueVertexIndices.size() + newVertices.size() > maxVertices)) {
                meshlets.push_back(current);
                current = Meshlet();
                globalToLocal.clear();
            }

            for (int j = 0; j < 3; ++j) {
                uint32_t globalIdx = tri[j];
                if (globalToLocal.find(globalIdx) == globalToLocal.end()) {
                    uint32_t localIdx = static_cast<uint32_t>(current.uniqueVertexIndices.size());
                    globalToLocal[globalIdx] = localIdx;
                    current.uniqueVertexIndices.push_back(globalIdx);
                }
                current.localIndices.push_back(globalToLocal[globalIdx]);
            }
        }

        if (!current.localIndices.empty()) {
            meshlets.push_back(current);
        }

        return meshlets;
	}

	void VirtualMeshBuilder::ExportMeshletsAsOBJ(const std::vector<Meshlet>& meshlets, const std::vector<Vertex>& vertices, const std::string& filename) {
        std::ofstream out(filename);
        if (!out.is_open()) return;

        size_t globalOffset = 1;
        for (size_t m = 0; m < meshlets.size(); ++m) {
            const auto& meshlet = meshlets[m];
            out << "o meshlet_" << m << "\n";

            for (uint32_t idx : meshlet.uniqueVertexIndices) {
                const auto& v = vertices[idx];
                out << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
            }

            for (size_t i = 0; i < meshlet.localIndices.size(); i += 3) {
                uint32_t i0 = meshlet.localIndices[i + 0];
                uint32_t i1 = meshlet.localIndices[i + 1];
                uint32_t i2 = meshlet.localIndices[i + 2];
                out << "f "
                    << (i0 + globalOffset) << " "
                    << (i1 + globalOffset) << " "
                    << (i2 + globalOffset) << "\n";
            }

            globalOffset += meshlet.uniqueVertexIndices.size();
        }

        out.close();
	}

    std::vector<Meshlet> VirtualMeshBuilder::BuildLODLevel(const std::vector<Meshlet>& previousLevel, size_t groupSize) {
        std::vector<Meshlet> lodLevel;
        for (size_t i = 0; i < previousLevel.size(); i += groupSize) {
            Meshlet merged;
            std::unordered_map<uint32_t, uint32_t> globalToLocal;
            size_t limit = std::min(groupSize, previousLevel.size() - i);

            for (size_t j = 0; j < limit; ++j) {
                const auto& src = previousLevel[i + j];
                for (uint32_t globalIdx : src.uniqueVertexIndices) {
                    if (globalToLocal.find(globalIdx) == globalToLocal.end()) {
                        uint32_t localIdx = static_cast<uint32_t>(merged.uniqueVertexIndices.size());
                        globalToLocal[globalIdx] = localIdx;
                        merged.uniqueVertexIndices.push_back(globalIdx);
                    }
                }

                for (uint32_t localIdx : src.localIndices) {
                    uint32_t globalIdx = src.uniqueVertexIndices[localIdx];
                    merged.localIndices.push_back(globalToLocal[globalIdx]);
                }
            }

            lodLevel.push_back(merged);
        }
        return lodLevel;
    }
}