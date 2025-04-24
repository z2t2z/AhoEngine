#pragma once

#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Camera/RuntimeCamera.h"
#include "Runtime/Resource/Asset/MeshAsset.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Resource/Asset/Animation/Animation.h"
#include "Runtime/Function/SkeletonViewer.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"
#include "Runtime/Core/BVH.h"
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
		MaterialMaskEnum* matMask{ nullptr };
		TextureHandlesComponent(TextureHandles* handle, MaterialMaskEnum* matMask) : handle(handle), matMask(matMask) {};
		TextureHandlesComponent() {
			handle = new TextureHandles();
			matMask = new MaterialMaskEnum();
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
}
