#pragma once

#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Camera/RuntimeCamera.h"
#include "Runtime/Resource/Asset/MeshAsset.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include <string>

namespace Aho {
	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};

	// To support mutiple mesh components
	struct EntityComponent {
		std::vector<entt::entity> entities;
		EntityComponent() = default;
		EntityComponent(const EntityComponent&) = default;
		EntityComponent(const std::vector<entt::entity>& _entities)
			: entities(_entities) {}
	};

	struct TransformComponent {
		TransformParam* transformPara;
		TransformComponent() { transformPara = new TransformParam(); }
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(TransformParam* t) : transformPara(t) {}
		glm::mat4 GetTransform() { return transformPara->GetTransform(); }
		glm::vec3& GetTranslation() { return transformPara->Translation; }
		glm::vec3& GetScale() { return transformPara->Scale; }
		glm::vec3& GetRotation() { return transformPara->Rotation; }
		void SetTranslation(glm::vec3 translation) { transformPara->Translation = translation; }
		void SetScale(glm::vec3 scale) { transformPara->Scale = scale; }
		void SetRotation(glm::vec3 rotation) { transformPara->Rotation = rotation; }
	};

	struct MaterialComponent {
		std::shared_ptr<Material> material;
		MaterialComponent() = default;
		MaterialComponent(std::shared_ptr<Material>& _material)
			: material(_material) {}
		MaterialComponent(const MaterialComponent&) = default;
	};

	// Temporary, consider how to design this
	struct MeshComponent {
		uint32_t meshID{ 0u };
		std::shared_ptr<VertexArray> vertexArray;
		MeshComponent() = default;
		MeshComponent(const std::shared_ptr<VertexArray>& _vertexArray, const uint32_t& id)
			: vertexArray(_vertexArray), meshID(id) {}
		MeshComponent(const MeshComponent&) = default;
	};
	
	struct MultiMeshComponent {
		std::vector<MeshComponent> meshes;
		MultiMeshComponent() = default;
		MultiMeshComponent(const std::vector<MeshComponent>& _meshes)
			: meshes(_meshes) {}
		MultiMeshComponent(const MultiMeshComponent&) = default;
	};

	struct AnimatorComponent {
		std::vector<BoneInfo> boneInfo;
		AnimatorComponent() = default;
		void Update(float deltaTime) {

		}
	};


	struct SkeletalMeshComponent {
		uint32_t meshID{ 0u };
		std::shared_ptr<VertexSkeletal> vertexArray;
		SkeletalMeshComponent() = default;
		SkeletalMeshComponent(const std::shared_ptr<VertexSkeletal>& _vertexArray, const uint32_t& id)
			: vertexArray(_vertexArray), meshID(id) {}
		SkeletalMeshComponent(const SkeletalMeshComponent&) = default;
	};

	// Temporary, think about how to design light class
	struct PointLightComponent {
		glm::vec4 color{ 0.1f, 0.12f, 0.15f, 1.0f };
		float intensity{ 0.5f };
		int count{ 0 };
		PointLightComponent() = default;
		PointLightComponent(glm::vec4 _color, float _intensity = 1.0f) 
			: color(_color), intensity(_intensity) {}
		PointLightComponent(const PointLightComponent&) = default;
	};
}
