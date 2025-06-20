#pragma once

#include "Runtime/Core/Core.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Aho {
	class Camera {
	public:
		virtual ~Camera() = default;
		virtual glm::vec3 GetPosition() const = 0;
		virtual const glm::mat4& GetProjection() = 0;
		virtual const glm::mat4& GetView() = 0;
		virtual const glm::mat4& GetProjectionInv() = 0;
		virtual const glm::mat4& GetViewInv() = 0;
		virtual glm::vec3 GetFront() const = 0;
		virtual glm::vec3 GetRight() const = 0;

		virtual void SetFOV(float fov) = 0;
		virtual float GetFOV() const = 0;
		virtual void SetAspectRatio(float ratio) = 0;
		virtual float GetAspectRatio() const = 0;
		virtual const float GetMoveSpeed() const = 0;
		virtual const float GetRotationSpeed() const = 0;

		virtual void SetProjection(const glm::mat4& projection) = 0;
		virtual void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) = 0;
		virtual void SetForwardRotation(const glm::quat& q) = 0;

		virtual void Update(float deltaTime, glm::vec3& movement) = 0;

		virtual void MoveForward(float deltaTime)	= 0;
		virtual void MoveBackward(float deltaTime)	= 0;
		virtual void MoveLeft(float deltaTime)		= 0;
		virtual void MoveRight(float deltaTime)		= 0;

		virtual bool GetUseMomentum() { return m_UseMomentum; }
		virtual void SetUseMomentum(bool state) { m_UseMomentum = state; }
	protected:
		bool m_ViewDirty{ true };
		bool m_ProjDirty{ true };
		bool m_UseMomentum{ true };
	};
}