#include "Ahopch.h"
#include "OpenGLShader.h"
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

	OpenGLShader::OpenGLShader(const std::string& filepath) : m_FilePath(filepath) {
		std::string source = ReadFile(filepath);
		if (source.empty()) {
			return;
		}
		m_OpenGLSourceCode = PreProcess(source);
		// Extract name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos ? 0 : lastSlash + 1);
		auto lastDot = filepath.rfind('.');
		auto count = (lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash);
		m_Name = filepath.substr(lastSlash, count);
		CompileFromSource();
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
		m_Name = name;
		m_OpenGLSourceCode[GL_VERTEX_SHADER] = vertexSrc;
		m_OpenGLSourceCode[GL_FRAGMENT_SHADER] = fragmentSrc;
		CompileFromSource();
	}

	OpenGLShader::~OpenGLShader() {
		glDeleteProgram(m_ShaderID);
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

	std::string OpenGLShader::ReadFile(const std::string& filepath) {
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in) {
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1) {
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else {
				AHO_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else {
			AHO_CORE_ERROR("Could not open file '{0}'", filepath);
		}
		if (result.empty()) {
			AHO_CORE_ERROR("No content from file:" + filepath);
		}
		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) {
		std::unordered_map<GLenum, std::string> shaderSources;
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			AHO_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			AHO_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			AHO_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line
			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}
		return shaderSources;
	}

	void OpenGLShader::CompileFromSource() {
		bool ComputeFlag = false;
		bool NormalFlag = false;
		std::vector<GLuint> shaderHandles;
		for (const auto& [shaderType, Source] : m_OpenGLSourceCode) {
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
				AHO_CORE_ERROR("Shader compilation failed in path {0}.{1}", m_FilePath, infoLog.data());
				return;
			}
			shaderHandles.push_back(shader);
		}

		AHO_CORE_ASSERT(!(ComputeFlag && NormalFlag), "Can not have normal shader and compute shader at the same time.");

		m_ShaderID = glCreateProgram();
		GLuint program = m_ShaderID;
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
			return;
		}
		// Always detach shaders after a successful link.
		for (const auto& handle : shaderHandles) {
			glDeleteShader(handle);
		}
		m_Compiled = true;
	}

	void OpenGLShader::CreateProgram() {
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV) {
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE) {
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog(maxLength);
			if (maxLength) {
				glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			}
			AHO_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());
			glDeleteProgram(program);
			for (auto id : shaderIDs) {
				glDeleteShader(id);
			}
		}
		for (auto id : shaderIDs) {
			glDetachShader(program, id);
			glDeleteShader(id);
		}
		m_ShaderID = program;
	}

	void OpenGLShader::SetUBO(size_t size, uint32_t bindingPoint, DrawType type) {
		m_BindingPoint = bindingPoint;
		glGenBuffers(1, &m_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLShader::BindUBO(const void* ubo, size_t size) {
		glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	/* Hard Code for now */

	void OpenGLShader::SetBool(const std::string& name, bool value) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::SetUint(const std::string& name, uint32_t value) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform1ui(location, value);
	}

	void OpenGLShader::SetInt(const std::string& name, int value) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::SetVec2(const std::string& name, const glm::vec2& value) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::SetVec3(const std::string& name, const glm::vec3& value) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::SetVec4(const std::string& name, const glm::vec4& value) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::SetMat2(const std::string& name, const glm::mat2& matrix) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix) {
		GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const {
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
		// TODO: customizable flags
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // Ensure the compute shader finishes
	}
}