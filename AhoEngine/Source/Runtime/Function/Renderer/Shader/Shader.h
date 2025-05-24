#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Aho {
	enum class ShaderType {
		None = 0,
		Normal, // vertex + pixel
		Compute
	};

	enum class DrawType {
		Static = 0,
		Dynamic
	};

	enum class ShaderUsage {
		None = 0, 
		Deferred,
		PathTracing,
	};

	class Shader {
	public:
		virtual ~Shader() = default;
        virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Delete() const = 0;
        // Uniforms
		virtual void SetBool(const std::string& name, bool value) const = 0;
		virtual void SetUint(const std::string& name, uint32_t value) const = 0;
        virtual void SetInt(const std::string& name, int value) const = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) const = 0;
        virtual void SetFloat(const std::string& name, float value) const = 0;
        virtual void SetVec2(const std::string& name, const glm::vec2& value) const = 0;
		virtual void SetIvec2(const std::string& name, const glm::ivec2& value) const = 0;
        virtual void SetVec3(const std::string& name, const glm::vec3& value) const = 0;
        virtual void SetVec4(const std::string& name, const glm::vec4& value) const = 0;
        virtual void SetMat2(const std::string& name, const glm::mat2& mat) const = 0;
        virtual void SetMat3(const std::string& name, const glm::mat3& mat) const = 0;
        virtual void SetMat4(const std::string& name, const glm::mat4& mat) const = 0;
		virtual bool IsCompiled() const { return m_Compiled; }
		// For compute shader
		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const = 0;

		virtual ShaderType GetShaderType() { return m_Type; }
		inline uint32_t GerRendererID() { return m_ShaderID; }

        virtual const std::string& GetName() const = 0;
		virtual bool Reload(const std::string& path) = 0;

        static std::shared_ptr<Shader> Create(const std::filesystem::path& filepath);
		static std::shared_ptr<Shader> Create(const std::string& name, const std::string& VertSrc, const std::string& fragSrc);
	protected:
		bool m_Compiled{ false };
		ShaderType m_Type{ ShaderType::None };
		uint32_t m_ShaderID{ 0u };
	};

}