#pragma once

#include "Core/Core.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace Aho {
	class AHO_API Camera {
	public:
		Camera() : m_Projection(glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f)) {}
		Camera(const glm::mat4& projection) : m_Projection(projection) {}

		inline const glm::mat4 GetProjection() const { return m_Projection; }
		inline void SetProjection(const glm::mat4& projection) { m_Projection = projection; }

	private:
		glm::mat4 m_Projection;
	};


}