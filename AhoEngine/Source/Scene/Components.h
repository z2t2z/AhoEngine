#pragma once

#include "Core/Camera/Camera.h"
#include "Core/Camera/RuntimeCamera.h"
#include "Core/Model/Model.h"
#include "Scene/Asset/MeshAsset.h"
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aho{
	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
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

	//struct RuntimeCameraComponent {
	//	RuntimeCamera* camera{ nullptr };
	//	bool Primary{ false };

	//	RuntimeCameraComponent() = default;
	//	RuntimeCameraComponent(const RuntimeCameraComponent&) = default;
	//	RuntimeCameraComponent(RuntimeCamera* _camera, bool primary) : camera(_camera), Primary(primary) {}
	//};

	// Temporary, consider how to design this
	struct MeshComponent {
		std::shared_ptr<VertexArray> vertexArray;

		MeshComponent() = default;
		MeshComponent(std::shared_ptr<VertexArray>& _vertexArray)
			: vertexArray(_vertexArray) {}
		MeshComponent(const MeshComponent&) = default;
	};

	struct MeshesComponent {
		std::shared_ptr<MeshAsset> meshAsset;
		MeshesComponent() = default;
		MeshesComponent(const std::shared_ptr<MeshAsset> _meshAsset)
			: meshAsset(_meshAsset) {}
		MeshesComponent(const MeshesComponent&) = default;
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
