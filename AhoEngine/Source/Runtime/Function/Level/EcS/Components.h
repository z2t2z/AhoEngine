#pragma once

#include "Runtime/Core/Geometry/BVH.h"

#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Camera/RuntimeCamera.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"
#include "Runtime/Function/Renderer/Lights.h"
#include "Runtime/Function/Renderer/RenderData.h"

#include "Runtime/Resource/Asset/Mesh/MeshAsset.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"

#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"
#include <string>

namespace Aho {
	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};

	struct RootComponent {
		std::vector<entt::entity> entities;
		RootComponent() = default;
		RootComponent(const RootComponent&) = default;
		RootComponent(const std::vector<entt::entity>& _entities)
			: entities(_entities) {
		}
	};

	struct BVHComponent {
		std::vector<BVHi*> bvhs;
		BVHComponent() = default;
		BVHComponent(const BVHComponent&) = default;
		BVHComponent(const std::vector<BVHi*>& bvhs)
			: bvhs(bvhs) {
		}
	};

	struct MeshComponent {
		std::shared_ptr<RenderData> renderData;
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const std::shared_ptr<RenderData>& renderData)
			: renderData(renderData) {
		}
	};

	struct TransformComponent {
		TransformParam* transformPara{ nullptr };
		TransformComponent(const TransformComponent&) = default;
		//~TransformComponent() { delete transformPara; }
		TransformComponent(TransformParam* t) : transformPara(t) {}
		glm::mat4 GetTransform() { return transformPara->GetTransform(); }
		glm::vec3& GetTranslation() { return transformPara->Translation; }
		glm::vec3& GetScale() { return transformPara->Scale; }
		glm::vec3& GetRotation() { return transformPara->Rotation; }
		bool dirty{ true };
		void SetTranslation(glm::vec3 translation) { transformPara->Translation = translation; }
		void SetScale(glm::vec3 scale) { transformPara->Scale = scale; }
		void SetRotation(glm::vec3 rotation) { transformPara->Rotation = rotation; }
	};

	struct MaterialComponent {
		std::shared_ptr<Material> material;
		// For path tracing 
		int32_t meshId{ -1 };
		MaterialComponent() = default;
		MaterialComponent(std::shared_ptr<Material>& _material, int32_t meshid)
			: material(_material), meshId(meshid) { }
		MaterialComponent(const MaterialComponent&) = default;
	};

	// For path tracing 
	struct TextureHandlesComponent {
		TextureHandles* handle{ nullptr };
		TextureHandlesComponent(TextureHandles* handle) 
			: handle(handle) {};
		TextureHandlesComponent() {
			handle = new TextureHandles();
		}
		TextureHandlesComponent(const TextureHandlesComponent&) = default;
	};

	struct AnimatorComponent {
		std::string name;
		int boneOffset;
		float currentTime;
		std::vector<glm::mat4> globalMatrices;
		AnimatorComponent(size_t boneCnt) : currentTime{ 0.0f }, name{ "AnimatorComponent" } {
			globalMatrices.resize(boneCnt);
			std::fill(globalMatrices.begin(), globalMatrices.end(), glm::mat4(1.0f));
		}
		static int s_BoneOffset;
	};

	struct AnimationComponent {
		std::string name;
		std::shared_ptr<AnimationAsset> animation;
		AnimationComponent(const std::shared_ptr<AnimationAsset>& _animation) : animation(_animation), name("IAmAnimation") {}
	};

	// Temporary, think carefully how to design this
	struct SkeletalComponent {
		std::string name;
		BoneNode* root;
		int offset;
		~SkeletalComponent() { delete root; }
		SkeletalComponent(BoneNode* _root, int _offset)
			: root(_root), offset(_offset), name{ _root->bone.name } {
		}
		SkeletalComponent(const SkeletalComponent&) = default;
	};

	struct BoneComponent {
		std::string name;
		BoneComponent(const std::string& _name) : name(_name) {}
	};

	struct BonesComponent {
		std::vector<BoneComponent> bones;
		BonesComponent() = default;
		BonesComponent(const std::vector<BoneComponent>& _bones) : bones(_bones) {}
	};

	class SkeletonViewer;
	struct SkeletonViewerComponent {
		std::string name;
		SkeletonViewer* viewer;
		SkeletonViewerComponent(SkeletonViewer* _viewer) : viewer(_viewer), name{ "Default" } {}
		~SkeletonViewerComponent() { delete viewer; }
	};

	// Temporary, think about how to design light class
	struct PointLightComponent {
		int index;
		glm::vec4 color{ 0.1f, 0.12f, 0.15f, 1.0f };
		bool castShadow{ false };
		PointLightComponent() = default;
		PointLightComponent(glm::vec4 _color)
			: color(_color) {
		}
		PointLightComponent(const PointLightComponent&) = default;
		static int s_PointLightCnt;
	};

	struct LightComponent {
		std::shared_ptr<Light> light;
		LightComponent() = default;
		LightComponent(const std::shared_ptr<Light>& light)
			: light(light) {
		}
		LightComponent(const LightComponent&) = default;
	};

	struct SkyComponent {
		glm::vec3 Color{ 1 };
		float Intensity{ 1 };
		glm::vec3 Direction;
		glm::vec3 DirectionXYZ;
		SkyComponent() {
			Direction = glm::vec3(45.0f, 0.0f, 0.0f);
			float theta = glm::radians(Direction.x), phi = glm::radians(Direction.y);
			DirectionXYZ = normalize(glm::vec3(glm::sin(theta) * glm::cos(phi), glm::cos(theta), glm::sin(theta) * glm::sin(phi)));
		}
		SkyComponent(const SkyComponent&) = default;
	};

	struct TextureComponent {
		TextureComponent() = default;
		TextureComponent(const Texture* tex) : texture(tex) {}
		TextureComponent(const TextureComponent&) = default;
		const Texture* texture{ nullptr };
	};

	// Temporary, think about how to design light class
	struct EnvComponent {
		std::vector<const Texture*> envTextures;
		EnvComponent() = default;
		EnvComponent(const Texture* tex) {
			envTextures.push_back(tex);
		}
		EnvComponent(const EnvComponent&) = default;
	};

	struct _TransformComponent {
		glm::vec3 Translation{ 0.0f };
		glm::vec3 Scale{ 1.0f };
		glm::vec3 Rotation{ 0.0f };
		_TransformComponent() = default;
		_TransformComponent(const _TransformComponent&) = default;
		explicit _TransformComponent(const glm::mat4& tf) {
			DecomposeTransform(tf, Translation, Rotation, Scale);
		}
		glm::mat4 GetTransform() const {
			return ComposeTransform(Translation, Rotation, Scale);
		}
	};

	// Root of meshes
	struct GameObjectComponent {
		std::string name;
		explicit GameObjectComponent(const std::string& s) : name(s) {}
	};

	// Asset Component
	struct AssetComponent {
		std::shared_ptr<Asset> asset;
		explicit AssetComponent(const std::shared_ptr<Asset>& asset) : asset(asset) {}
		AssetComponent(const AssetComponent&) = default;
	};
	struct MeshAssetComponent {
		std::shared_ptr<MeshAsset> asset;
		explicit MeshAssetComponent(const std::shared_ptr<MeshAsset>& asset) : asset(asset) {}
		MeshAssetComponent(const MeshAssetComponent&) = default;
	};
	struct TextureAssetComponent {
		std::shared_ptr<TextureAsset> asset;
		explicit TextureAssetComponent(const std::shared_ptr<TextureAsset>& asset) : asset(asset) {}
		TextureAssetComponent(const TextureAssetComponent&) = default;
	};
	struct MaterialAssetComponent {
		std::shared_ptr<MaterialAsset> asset;
		explicit MaterialAssetComponent(const std::shared_ptr<MaterialAsset>& asset) : asset(asset) {}
		MaterialAssetComponent() = default;
		MaterialAssetComponent(const MaterialAssetComponent&) = default;
	};

	struct VertexArrayRefsComponent {
		std::vector<entt::entity> vaoRef;
		VertexArrayRefsComponent() = default;
	};
	// For rendering
	struct VertexArrayComponent {
		VertexArray* vao;
		VertexArrayComponent(VertexArray* vao)
			: vao(vao) { }
	};
	//struct RenderableComponent {
	//	_Material* mat{ nullptr };
	//	VertexArray* vao{ nullptr };
	//};
	struct _MaterialComponent {
		_Material mat;
		_MaterialComponent(const _Material& mat) : mat(mat) {}
		_MaterialComponent(const std::shared_ptr<MaterialAsset>& matAsset) 
			: mat(matAsset) {}
	};
	struct MeshResourceComponent {
		std::string name{ "MeshResourceComponent" };
		MeshResourceComponent() = default;
	};
	struct AssetRefComponent {
		Entity AssetRefEntity;
		explicit AssetRefComponent(const Entity& AssetRefEntity) : AssetRefEntity(AssetRefEntity) {}
		AssetRefComponent(const AssetRefComponent&) = default;
	};
	struct DanglingComponent {
		bool IsDangling;
		explicit DanglingComponent() : IsDangling(true) {}
	};
	struct ReferenceComponent {
		size_t ReferenceCount;
		explicit ReferenceComponent() : ReferenceCount(0) {}
	};
	struct MaterialRefComponent {
		Entity MaterialRefEntity;
		explicit MaterialRefComponent(const Entity& MaterialRefEntity) : MaterialRefEntity(MaterialRefEntity) {}
		MaterialRefComponent(const MaterialRefComponent&) = default;
	};

	// Temp, consider light class
	struct DistantLightComponent {
		DistantLightComponent() = default;
		glm::vec3 LightDir;
		glm::vec3 LightAlbedo{ 1.0f };
		float Intensity{ 1.0f };
		bool CastShadow{ true };
	};
	struct AtmosphereParametersComponent {
		AtmosphereParametersComponent() = default;

		// Radius of the planet (center to ground)
		float BottomRadius{ 6360.0f };
		// Maximum considered atmosphere height (center to atmosphere top)
		float TopRadius{ 6460.0f };

		// Rayleigh scattering exponential distribution scale in the atmosphere
		float RayleighDensityExpScale;
		// Rayleigh scattering coefficients
		glm::vec3 RayleighScattering;

		// Mie scattering exponential distribution scale in the atmosphere
		float MieDensityExpScale;
		// Mie scattering coefficients
		glm::vec3 MieScattering;
		// Mie extinction coefficients
		glm::vec3 MieExtinction;
		// Mie absorption coefficients
		glm::vec3 MieAbsorption;
		// Mie phase function excentricity
		float MiePhaseG;

		// An atmosphere layer of width 'width', and whose density is defined as
		// 'ExpTerm' * exp('ExpScale' * h) + 'LinearTerm' * h + 'ConstantTerm',
		// clamped to [0,1], and where h is the altitude.	
		// Refer to Bruneton's implementation of definitions.glsl for more details
		// https://github.com/sebh/UnrealEngineSkyAtmosphere/blob/183ead5bdacc701b3b626347a680a2f3cd3d4fbd/Resources/Bruneton17/definitions.glsl
		glm::vec3 AbsorptionExtinction;
		float Width0;
		float ExpTerm0;
		float ExpScale0;
		float LinearTerm0;
		float ConstantTerm0;

		float Width1;
		float ExpTerm1;
		float ExpScale1;
		float LinearTerm1;
		float ConstantTerm1;

		// The albedo of the ground.
		glm::vec3 GroundAlbedo{ 1.0f };
	};
}
