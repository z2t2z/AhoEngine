#pragma once

#include <glm/glm.hpp>

namespace Aho {
	struct alignas(16) Vertex {
		glm::vec3 position;		float u;
		glm::vec3 normal;		float v;
		glm::vec3 tangent;		float _padding{ -1.0f };
		Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& uv)
			: position(pos), normal(normal), tangent(tangent), u(uv.x), v(uv.y) {
		}
		Vertex() = default;
	};

	constexpr int MAX_BONES = 4;
	struct VertexSkeletal {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec2 uv;

		VertexSkeletal(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& uv)
			: position(pos), normal(normal), tangent(tangent), uv(uv) {
			memset(bonesID, -1, sizeof(bonesID));
			memset(weights, 0, sizeof(weights));
		}

		float weights[MAX_BONES];
		int bonesID[MAX_BONES];
		VertexSkeletal() {
			memset(bonesID, -1, sizeof(bonesID));
			memset(weights, 0, sizeof(weights));
		}
	};
}
