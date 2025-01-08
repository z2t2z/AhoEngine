#pragma once

#include "Runtime/Core/Math/Math.h"

namespace Aho {
	struct Ray {
		Ray() = default;
		Ray(const glm::vec3& origin, const glm::vec3& direction)
			: origin(origin), direction(direction) {
		}
		glm::vec3 origin;		float _padding0;
		glm::vec3 direction;	float _padding1;
	};

	struct IntersectResult {
		IntersectResult(float t, const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& baryC, const glm::vec2& uv)
			: t(t), position(pos), normal(normal), baryCentric(baryC), uv(uv) {
		}
		float t;

		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 baryCentric;
		glm::vec2 uv;
		glm::vec4 albedo;
	};
}
