#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"

#include <glad/glad.h>

namespace Aho {
	class OpenGLShader : public Shader {
	public:
		OpenGLShader(const std::string& filepath) {
			AHO_CORE_ASSERT(false);
		}
		OpenGLShader() = default;
		virtual ~OpenGLShader();
		virtual bool TryCompile(const std::unordered_map<uint32_t, std::string>& src) override;
		virtual void Delete() const override;
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const override;
	public:
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
	private:
		uint32_t TryCompileFromSource(const std::unordered_map<GLenum, std::string>& src);
	private:
		bool m_IsCompute{ false };
	private:
		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};

}