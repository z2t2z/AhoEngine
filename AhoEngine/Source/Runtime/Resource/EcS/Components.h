#pragma once

#include "Runtime/Function/Camera/Camera.h"
#include "Runtime/Function/Camera/RuntimeCamera.h"
#include "Runtime/Resource/Model/Model.h"
#include "Runtime/Resource/Asset/MeshAsset.h"
#include "Runtime/Resource/EcS/AObject.h"
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aho {
	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};

	struct EntityComponent {
		std::vector<entt::entity> meshEntities;
		EntityComponent() = default;
		EntityComponent(const EntityComponent&) = default;
		EntityComponent(const std::vector<entt::entity>& _meshEntities)
			: meshEntities(_meshEntities) {}
	};


	struct TransformComponent {
		glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation) : Translation(translation) {}

		glm::mat4 GetTransform() const {
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
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
		std::shared_ptr<VertexArray> vertexArray;
		MeshComponent() = default;
		MeshComponent(const std::shared_ptr<VertexArray>& _vertexArray)
			: vertexArray(_vertexArray) {}
		MeshComponent(const MeshComponent&) = default;
	};
	
	struct MultiMeshComponent {
		std::vector<MeshComponent> meshes;
		MultiMeshComponent() = default;
		MultiMeshComponent(const std::vector<MeshComponent>& _meshes)
			: meshes(_meshes) {}
		MultiMeshComponent(const MultiMeshComponent&) = default;
	};

	// Temporary, think about how to design light class
	struct PointLightComponent {
		glm::vec3 color;
		float intensity;

		PointLightComponent() = default;
		PointLightComponent(glm::vec3 _color, float _intensity = 1.0f) 
			: color(_color), intensity(_intensity) {}
		PointLightComponent(const PointLightComponent&) = default;
	};

}
