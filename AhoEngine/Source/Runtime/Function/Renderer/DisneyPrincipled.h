#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <unordered_map>

namespace Aho {
	struct alignas(16) MaterialDescriptor {
		uint64_t albedoHandle{ 0 };
		uint64_t normalHandle{ 0 };

		uint64_t metallicHandle{ 0 };
		uint64_t roughnessHandle{ 0 };

		glm::vec3 baseColor{ 0.0, 0.0, 1.0 };
		float subsurface{ 0 };

		glm::vec3 emissive{ 0.0 };
		float emissiveScale{ 0.0f };

		float metallic{ 0.1 };
		float specular{ 0 };
		float specTint{ 0 };
		float roughness{ 0.5 };

		float anisotropic{ 0 };
		float sheen{ 0 };
		float sheenTint{ 0 };
		float clearcoat{ 0 };

		float clearcoatGloss{ 0 };
		float specTrans{ 0 };
		float ior{ 1.5 };
		float alpha_x;

		float alpha_y;
		float ao{ 0.0f };
		float padding0;
		float padding1;

		static inline void CalDistParams(float anisotropic, float roughness, float& ax, float& ay) {
			float roughness2 = roughness * roughness;
			if (anisotropic == 0) {
				ax = std::max(0.0001f, roughness2);
				ay = ax;
				return;
			}
			float aspect = sqrt(1.0 - 0.9 * anisotropic);
			ax = std::max(0.0001f, roughness2 / aspect);
			ay = std::max(0.0001f, roughness2 * aspect);
		}
	};

}