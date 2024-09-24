#pragma once

#include "Core/Core.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Aho {
	//class AHO_API Camera {
	//public:
	//	// TODO : Customizable camera properties
	//	Camera() : m_Projection(glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f)) {}
	//	Camera(const glm::mat4& projection) : m_Projection(projection) {}

	//	inline const glm::mat4 GetProjection() const { return m_Projection; }
	//	inline void SetProjection(const glm::mat4& projection) { m_Projection = projection; }

	//private:
	//	glm::mat4 m_Projection;
	//};
	class AHO_API Camera {
	public:
		virtual ~Camera() = default;
		virtual const glm::vec3& GetPosition() const = 0;
		virtual const glm::mat4& GetProjection() const = 0;
		virtual const glm::mat4& GetView() const = 0;
		virtual void SetProjection(const glm::mat4& projection) = 0;
		virtual void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) = 0;
		virtual void Update(float deltaTime, glm::vec3& movement) = 0;

		virtual void MoveForward(float deltaTime)	= 0;
		virtual void MoveBackward(float deltaTime)	= 0;
		virtual void MoveLeft(float deltaTime)		= 0;
		virtual void MoveRight(float deltaTime)		= 0;
	};
	

}