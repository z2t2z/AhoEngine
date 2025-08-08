#pragma once

#include "Runtime/Core/Geometry/BVH.h"
#include "Runtime/Core/Geometry/Mesh.h"

#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Camera/RuntimeCamera.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"
#include "Runtime/Function/Renderer/Lights.h"
#include "Runtime/Function/Renderer/RenderData.h"
#include "Runtime/Function/Renderer/IBL/IBL.h"
#include "Runtime/Function/Renderer/Texture.h"

#include "Runtime/Resource/Asset/Mesh/MeshAsset.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"
#include "Runtime/Resource/Asset/ShaderAsset.h"

#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"
#include <string>

namespace Aho {
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

	struct DirectionalLightComponent {
		std::shared_ptr<DirectionalLight> light;
		uint32_t index;
		DirectionalLightComponent() = default;
		DirectionalLightComponent(const std::shared_ptr<DirectionalLight>& light)
			: light(light) {
		}
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	struct AreaLightComponent {
		std::shared_ptr<AreaLight> light;
		uint32_t index;
		AreaLightComponent() = default;
		AreaLightComponent(const std::shared_ptr<AreaLight>& light)
			: light(light) {
		}
		AreaLightComponent(const AreaLightComponent&) = default;
	};

	struct LightComponent {
		std::shared_ptr<Light> light;
		uint32_t index;
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

	struct _TransformComponent {
		bool Dirty{ false };
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

	struct EditorSelectedTag {
		bool tag;
		EditorSelectedTag() = default;
	};

	// Root of meshes
	struct GameObjectComponent {
		std::string name;
		Entity parent;
		std::vector<Entity> children;
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
	struct ShaderAssetComponent {
		std::shared_ptr<ShaderAsset> asset;
		explicit ShaderAssetComponent(const std::shared_ptr<ShaderAsset>& asset) : asset(asset) {}
		ShaderAssetComponent(const ShaderAssetComponent&) = default;
	};

	// Resource Components
	struct ShaderResourceComponent {
		std::shared_ptr<ShaderAsset> shaderAsset;
		std::shared_ptr<Shader> shader;
		ShaderFeature feature = ShaderFeature::NONE;
		ShaderUsage usage;
		explicit ShaderResourceComponent(const std::shared_ptr<ShaderAsset>& shaderAsset, const std::shared_ptr<Shader>& shaderResource, ShaderUsage usage)
			: shaderAsset(shaderAsset), shader(shaderResource), usage(usage) {}
		ShaderResourceComponent(const ShaderResourceComponent&) = default;
	};

	struct VertexArrayRefsComponent {
		std::vector<entt::entity> vaoRef;
		VertexArrayRefsComponent() = default;
	};
	struct VertexArrayComponent {
		VertexArray* vao;
		explicit VertexArrayComponent(VertexArray* vao)
			: vao(vao) { }
	};
	struct _MaterialComponent {
		_Material mat;
		bool Dirty = true;
		explicit _MaterialComponent(const _Material& mat) : mat(mat) {}
		_MaterialComponent(const std::shared_ptr<MaterialAsset>& matAsset) 
			: mat(matAsset) {}
	};

	struct MaterialRefComponent {
		Entity MaterialRefEntity;
		explicit MaterialRefComponent(const Entity& MaterialRefEntity) : MaterialRefEntity(MaterialRefEntity) {}
		MaterialRefComponent(const MaterialRefComponent&) = default;
	};
	struct SceneBVHComponent {
		std::unique_ptr<BVHi> bvh{ nullptr };
		SceneBVHComponent() {
			bvh = std::make_unique<BVHi>(); // Scene TLAS BVH
		}
	};
	struct EditorCamaraComponent {
		bool Dirty{ true };
		std::shared_ptr<Camera> camera{ nullptr };
		EditorCamaraComponent() = default;
		explicit EditorCamaraComponent(const std::shared_ptr<Camera>& _camera)
			: camera(_camera) {
		}
	};
	struct InactiveComponent {
		bool Inactive{ true };
		InactiveComponent() = default;
		InactiveComponent(bool inactive) : Inactive(inactive) {}
	};

	struct _BVHComponent {
		bool Dirty = true;
		std::shared_ptr<BVHi> bvh{ nullptr };
		explicit _BVHComponent(const std::shared_ptr<BVHi>& _bvh) {
			bvh = _bvh;
		}
	};

	struct LightDirtyTagComponent {
		int placeholder;
		LightDirtyTagComponent() = default;
	};

	//struct IBLComponent;
	struct IBLComponent {
		std::unique_ptr<IBL> IBL{ nullptr };
		_Texture* EnvTexture{ nullptr }; // rectangular
		std::unique_ptr<_Texture> EnvTextureSkyBox{ nullptr };
		std::unique_ptr<_Texture> Prefilter{ nullptr };
		std::unique_ptr<_Texture> Irradiance{ nullptr };
		static std::unique_ptr<_Texture> BRDFLUT;
		int PrefilterBaseRes = 512;
		int SkyboxBaseRes = 512;
		int IrradianceRes = 64;
		int BRDFRes;
		IBLComponent() = default;
	};

	struct AtmosphereParametersComponent {
		AtmosphereParametersComponent() = default;

		uint32_t SkyViewTextureID{ 0 };
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
