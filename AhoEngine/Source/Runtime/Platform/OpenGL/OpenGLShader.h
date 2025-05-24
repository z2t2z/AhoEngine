#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"

#include <glad/glad.h>

namespace Aho {
	class OpenGLShader : public Shader {
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();
		virtual bool Reload(const std::string& path) override;
		virtual void Delete() const override;
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void SetBool(const std::string& name, bool value) const override;
		virtual void SetUint(const std::string& name, uint32_t value) const override;
		virtual void SetInt(const std::string& name, int value) const override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) const override;
		virtual void SetFloat(const std::string& name, float value) const override;
		virtual void SetVec2(const std::string& name, const glm::vec2& value) const override;
		virtual void SetIvec2(const std::string& name, const glm::ivec2& value) const override;
		virtual void SetVec3(const std::string& name, const glm::vec3& value) const override;
		virtual void SetVec4(const std::string& name, const glm::vec4& value) const override;
		virtual void SetMat2(const std::string& name, const glm::mat2& mat) const override;
		virtual void SetMat3(const std::string& name, const glm::mat3& mat) const override;
		virtual void SetMat4(const std::string& name, const glm::mat4& mat) const override;
		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const override;
		virtual const std::string& GetName() const override { return m_Name; }
	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void ReplaceIncludes(std::unordered_map<GLenum, std::string>& src);
		uint32_t CompileFromSource(const std::unordered_map<GLenum, std::string>& src);
		void CreateProgram();
	private:
		bool m_IsCompute{ false };
		std::string m_FilePath;
		std::string m_Name;
	private:
		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};

}