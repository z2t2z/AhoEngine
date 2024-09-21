#pragma once

#include "Core/Core.h"
#include "Shader.h"

#include <memory>

namespace Aho {
	class AHO_API Material {
	public:
		Material(std::string& filepath) { m_Shader = Shader::Create(filepath); }
		~Material() = default;

		void SetShader(std::shared_ptr<Shader>& shader) { m_Shader = shader; }
		void SetInt(const std::string& name, int value) { m_Shader->SetInt(name, value); }
		void SetIntArray(const std::string& name, int* values, uint32_t count) { m_Shader->SetIntArray(name, values, count); }
		void SetFloat(const std::string& name, float value) { m_Shader->SetFloat(name, value); }
		void SetVec2(const std::string& name, const glm::vec2& value) { m_Shader->SetVec2(name, value); }
		void SetVec3(const std::string& name, const glm::vec3& value) { m_Shader->SetVec3(name, value); }
		void SetVec4(const std::string& name, const glm::vec4& value) { m_Shader->SetVec4(name, value); }
		void SetMat2(const std::string& name, const glm::mat2& mat) { m_Shader->SetMat2(name, mat); }
		void SetMat3(const std::string& name, const glm::mat3& mat) { m_Shader->SetMat3(name, mat); }
		void SetMat4(const std::string& name, const glm::mat4& mat) { m_Shader->SetMat4(name, mat); }

		void Bind() const { m_Shader->Bind(); }

		std::shared_ptr<Shader> GetShader() { return m_Shader; }
	private:
		std::shared_ptr<Shader> m_Shader;
	};

}