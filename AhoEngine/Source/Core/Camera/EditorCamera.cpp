#include "Ahopch.h"
#include "Camera.h"
#include "EditorCamera.h"

#include <glm/glm.hpp>

namespace Aho {
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearPlane, float farPlane) 
			: m_Speed(10.0f),
			m_RotateSpeed(0.1f),
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
		//AHO_CORE_INFO("{}", m_AspectRatio);
		//AHO_CORE_INFO("{}", m_Fov);
		//AHO_CORE_INFO("{}", m_NearPlane);
		//AHO_CORE_INFO("{}", m_FarPlane);
		m_Position += glm::normalize(movement) * m_Speed * deltaTime;
		RecalculateViewMatrix();
	}

	void EditorCamera::RecalculateViewMatrix() {
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
	}
	
	void EditorCamera::RecalculateProjectionMatrix() {
		m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearPlane, m_FarPlane);
	}
}