#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Log/Log.h"
#include "Runtime/Function/Renderer/Texture.h"
#include <unordered_map>

//#define DEBUG

namespace Aho {

	struct alignas(16) TextureHandles {
		uint64_t albedoHandle{ 0 };
		uint64_t normalHandle{ 0 };

		uint64_t metallicHandle{ 0 };
		uint64_t roughnessHandle{ 0 };

#ifndef DEBUG
		glm::vec3 baseColor{ 1 };
		float subsurface{ 0 };

		glm::vec3 emissive{ 0.0 };
		float emissiveScale{ 0.0f };

		float metallic{ 0.1 };
		float specular{ 0 };
		float specTint{ 0 };
		float roughness{ 0 };

		float anisotropic{ 0 };
		float sheen{ 0 };
		float sheenTint{ 0 };
		float clearcoat{ 0 };

		float clearcoatGloss{ 0 };
		float specTrans{ 0 };
		float ior{ 1.5 };
		float alpha_x;

		float alpha_y;
		//glm::vec3 _padding;
		float padding0;
		float padding1;
		float padding2;
#endif

		void SetHandles(uint64_t handleId, TexType type) {
			AHO_CORE_ASSERT(handleId > 0);
			switch (type) {
			case TexType::Albedo:
				albedoHandle = handleId;
				break;
			case TexType::Normal:
				normalHandle = handleId;
				break;
			case TexType::Roughness:
				roughnessHandle = handleId;
				break;
			case TexType::Metallic:
				metallicHandle = handleId;
				break;
				AHO_CORE_WARN("Wrong texture type");
			}
		}

		template<typename T>
		void SetValue(T v, TexType type) {
#ifndef DEBUG
			if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
				AHO_CORE_ASSERT(false);
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec3>) {
				switch (type) {
				case TexType::Albedo:
					baseColor = v;
					break;
				case TexType::Emissive:
					emissive = v;
					break;
				}
				return;
			}
			else {
				switch (type) {
				case TexType::Subsurface:
					subsurface = v;
					break;
				case TexType::EmissiveScale:
					emissiveScale = v;
					break;
				case TexType::Metallic:
					metallic = v;
					break;
				case TexType::Specular:
					specular = v;
					break;
				case TexType::SpecTint:
					specTint = v;
					break;
				case TexType::Roughness:
					roughness = v;
					CalDistParams(anisotropic, roughness, alpha_x, alpha_y);
					break;
				case TexType::Anisotropic:
					anisotropic = v;
					CalDistParams(anisotropic, roughness, alpha_x, alpha_y);
					break;
				case TexType::Sheen:
					sheen = v;
					break;
				case TexType::SheenTint:
					sheenTint = v;
					break;
				case TexType::Clearcoat:
					clearcoat = v;
					break;
				case TexType::ClearcoatGloss:
					clearcoatGloss = v;
					break;
				case TexType::SpecTrans:
					specTrans = v;
					break;
				case TexType::ior:
					ior = v;
					break;
				default:
					AHO_CORE_ASSERT(false);
				}
			}
#endif
		}
		void CalDistParams(float anisotropic, float roughness, float& ax, float& ay) {
#ifndef  DEBUG
			float roughness2 = roughness * roughness;
			if (anisotropic == 0) {
				ax = std::max(0.0001f, roughness2);
				ay = ax;
				return;
			}
			float aspect = sqrt(1.0 - 0.9 * anisotropic);
			ax = std::max(0.0001f, roughness2 / aspect);
			ay = std::max(0.0001f, roughness2 * aspect);
#endif // ! DEBUG
		}
		static std::unordered_map<TexType, std::string> s_Umap;
	};

}