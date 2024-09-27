#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Aho {
	template <typename T>
	using Ref = std::shared_ptr<T>;

	enum class ShaderType {
		Normal, // vertex + pixel
		Compute,
		None
	};

	class AHO_API Shader {
	public:
		virtual ~Shader() = default;
        
        virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

        // Uniforms
        virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
        virtual void SetFloat(const std::string& name, float value) = 0;
        virtual void SetVec2(const std::string& name, const glm::vec2& value) = 0;
        virtual void SetVec3(const std::string& name, const glm::vec3& value) = 0;
        virtual void SetVec4(const std::string& name, const glm::vec4& value) = 0;
        virtual void SetMat2(const std::string& name, const glm::mat2& mat) = 0;
        virtual void SetMat3(const std::string& name, const glm::mat3& mat) = 0;
        virtual void SetMat4(const std::string& name, const glm::mat4& mat) = 0;

		// For compute shader
		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const = 0;

		virtual ShaderType GetShaderType() { return m_Type; }
		inline uint32_t GerRendererID() { return m_RendererID; }

        virtual const std::string& GetName() const = 0;
        static std::shared_ptr<Shader> Create(const std::string& filepath);
		static std::shared_ptr<Shader> Create(const std::string& name, const std::string& VertSrc, const std::string& fragSrc);
	private:
		ShaderType m_Type;
		uint32_t m_RendererID;
	};

	class AHO_API ShaderLibrary {
	public:
		void Add(const std::string& name, const std::shared_ptr<Shader>& shader);
		void Add(const std::shared_ptr<Shader>& shader);
		std::shared_ptr<Shader> Load(const std::string& filepath);
		std::shared_ptr<Shader> Load(const std::string& name, const std::string& filepath);

		std::shared_ptr<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	};

}

