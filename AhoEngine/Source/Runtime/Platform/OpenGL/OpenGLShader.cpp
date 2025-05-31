#include "Ahopch.h"
#include "OpenGLShader.h"
#include "Runtime/Resource/Asset/ShaderAsset.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	namespace Utils {
		static GLenum ShaderTypeFromString(const std::string& type) {
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;
			if (type == "compute")
				return GL_COMPUTE_SHADER;
			AHO_CORE_ASSERT(false, "Unknown shader type: " + type);
			return 0;
		}
	} // namespace Aho::Utils

	OpenGLShader::~OpenGLShader() {
		Delete();
	}

	bool OpenGLShader::TryCompile(const std::unordered_map<uint32_t, std::string>& src) {
		uint32_t shaderID = TryCompileFromSource(src);
		if (!shaderID) {
			AHO_CORE_ERROR("Shader compilation failed for source code provided.");
			return false;
		}
		if (m_ShaderID) {
			glDeleteProgram(m_ShaderID);
		}
		m_Compiled = true;
		m_ShaderID = shaderID;
		return true;
	}

	void OpenGLShader::Delete() const {
		glDeleteProgram(m_ShaderID);
	}

	void OpenGLShader::Bind() const {
		glUseProgram(m_ShaderID);
	}

	void OpenGLShader::Unbind() const {
		glUseProgram(0);
	}

	uint32_t OpenGLShader::TryCompileFromSource(const std::unordered_map<GLenum, std::string>& src) {
		bool ComputeFlag = false;
		bool NormalFlag = false;
		std::vector<GLuint> shaderHandles;
		for (const auto& [shaderType, Source] : src) {
			(shaderType == GL_COMPUTE_SHADER ? ComputeFlag : NormalFlag) = true;
			GLuint shader = glCreateShader(shaderType);
			const GLchar* source = Source.c_str();
			glShaderSource(shader, 1, &source, NULL);
			glCompileShader(shader);
			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE) {
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				if (maxLength) { // if maxlength is somehow 0 then there will be segment fault
					glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				}
				// We don't need the shader anymore.
				glDeleteShader(shader);
				AHO_CORE_ERROR("OpenGLShader::Shader compilation failed:\n {}", infoLog.data());
				return 0u;
			}
			shaderHandles.push_back(shader);
		}

		AHO_CORE_ASSERT(!(ComputeFlag && NormalFlag), "Can not have normal shader and compute shader at the same time.");

		GLuint program = glCreateProgram();
		for (const auto& handle : shaderHandles) {
			glAttachShader(program, handle);
		}
		glLinkProgram(program);
		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			if (maxLength) {
				glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
			}
			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			for (const auto& handle : shaderHandles) {
				glDeleteShader(handle);
			}
			AHO_CORE_ERROR("{0}", infoLog.data());
			//AHO_CORE_ASSERT(false, "Shader link failure!");
			return 0u;
		}
		// Always detach shaders after a successful link.
		for (const auto& handle : shaderHandles) {
			glDeleteShader(handle);
		}

		return program;
	}

	void OpenGLShader::SetBool(const std::string& name, bool value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform1i(location, value);
	}

	void OpenGLShader::SetUint(const std::string& name, uint32_t value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform1ui(location, value);
	}

	void OpenGLShader::SetInt(const std::string& name, int value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform1i(location, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform1f(location, value);
	}

	void OpenGLShader::SetVec2(const std::string& name, const glm::vec2& value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::SetIvec2(const std::string& name, const glm::ivec2& value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform2i(location, value.x, value.y);
	}

	void OpenGLShader::SetVec3(const std::string& name, const glm::vec3& value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::SetVec4(const std::string& name, const glm::vec4& value) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::SetMat2(const std::string& name, const glm::mat2& matrix) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix) const {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		if (location == -1) {
			//AHO_CORE_WARN("Could not find uniform: {}", name);
			return;
		}
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const {
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
		// TODO: customizable flags
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // Ensure the compute shader finishes
	}

}