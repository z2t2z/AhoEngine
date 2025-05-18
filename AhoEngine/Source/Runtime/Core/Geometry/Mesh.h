#pragma once

#include "Vertex.h"

namespace Aho {
	struct Mesh {
		std::vector<Vertex> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		std::string name;
		bool hasNormal{ false };
		bool hasUVs{ false };
		Mesh() = default;
		Mesh(const std::vector<Vertex>& _vertexBuffer, const std::vector<uint32_t>& _indexBuffer, const std::string& name, bool _hasNormal, bool _hasUV)
			: vertexBuffer(_vertexBuffer), indexBuffer(_indexBuffer), name(name), hasNormal(_hasNormal), hasUVs(_hasUV) {
		}
	};
}
