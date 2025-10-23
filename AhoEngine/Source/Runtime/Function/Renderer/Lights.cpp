#include "Ahopch.h"
#include "Lights.h"

#include <glm/glm.hpp>                   
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>          

namespace Aho {
	void DirectionalLight::CalProjViewMat() {
		// A virtual position is needed
		glm::vec3 lightPos = m_WorldCenter - (-m_Direction) * m_LightDistance;

		glm::mat4 lightView = glm::lookAt(lightPos, m_WorldCenter, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 lightProj = glm::ortho(-m_OrthoSize, m_OrthoSize, -m_OrthoSize, m_OrthoSize, m_Near, m_Far);

		m_ProjView = lightProj * lightView;
	}
}