#include "Ahopch.h"
#include "Camera.h"
#include "EditorCamera.h"

#include <glm/glm.hpp>

namespace Aho {
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearPlane, float farPlane) 
			: m_Speed(10.0f),
			m_RotateSpeed(1.0f),
			m_Fov(fov), 
			m_AspectRatio(aspectRatio), 
			m_NearPlane(nearPlane), 
			m_FarPlane(farPlane),
			m_Position{ 0.0f, 0.0f, 0.0f },
			m_Up{ 0.0f, 1.0f, 0.0f },
			m_Front{ 0.0f, 0.0f, -1.0f },
			m_Right{ 1.0f, 0.0f, 0.0f } {
		RecalculateViewMatrix();
		RecalculateProjectionMatrix();
	}

	void EditorCamera::SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) {
		m_Fov = fov;
		m_AspectRatio = aspectRatio;
		m_NearPlane = nearPlane;
		m_FarPlane = farPlane;
		RecalculateProjectionMatrix();
	}
	
	void EditorCamera::Update(float deltaTime, glm::vec3& movement) {
		if (glm::length(movement) == 0) {
			return;
		}

		//m_Position += glm::normalize(movement * m_Front) * m_Speed * deltaTime;
		m_Position.z += movement.z * m_Speed * deltaTime;
		m_Position.x += movement.x * m_Speed * deltaTime;
		RecalculateViewMatrix();
	}

	void EditorCamera::RecalculateViewMatrix() {
		m_Right = glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f))); 
		m_Up = glm::normalize(glm::cross(m_Right, m_Front));
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_ViewMatrixInv = glm::inverse(m_ViewMatrix);
	}
	
	void EditorCamera::RecalculateProjectionMatrix() {
		m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
		m_ProjectionMatrixInv = glm::inverse(m_ProjectionMatrix);
	}
}