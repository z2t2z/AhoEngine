#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/Shader.h"

#include <glad/glad.h>

namespace Aho {
	class OpenGLShader : public Shader {
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();
		virtual void Delete() const override;
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void BindUBO(const UBO& ubo) override;
		virtual void SetBool(const std::string& name, bool value) override;
		virtual void SetUint(const std::string& name, uint32_t value) override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetVec2(const std::string& name, const glm::vec2& value) override;
		virtual void SetVec3(const std::string& name, const glm::vec3& value) override;
		virtual void SetVec4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat2(const std::string& name, const glm::mat2& mat) override;
		virtual void SetMat3(const std::string& name, const glm::mat3& mat) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& mat) override;
		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const override;
		virtual const std::string& GetName() const override { return m_Name; }
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void CompileFromSource();
		void CreateProgram();
		void InitUBO();
	private:
		bool m_IsCompute = false;
		uint32_t m_UBO{ 0u };
		uint32_t m_BindingPoint{ 0u };
		std::string m_FilePath;
		std::string m_Name;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};

}