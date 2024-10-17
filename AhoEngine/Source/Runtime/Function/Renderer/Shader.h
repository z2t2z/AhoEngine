#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Aho {
	template <typename T>
	using Ref = std::shared_ptr<T>;

	enum class ShaderType {
		None = 0,
		Normal, // vertex + pixel
		Compute
	};

	// TODO: For now this is hardcoded for every shader
	constexpr int MAX_LIGHT_CNT = 4;
	struct UBO {
		UBO() {
			for (int i = 0; i < MAX_LIGHT_CNT; i++) {
				u_LightPosition[i] = u_LightColor[i] = glm::vec4(0.0f);
				u_LightPosition[i].w = 1.0f;
			}
			memset(info, 0, sizeof(info));
		}
		glm::vec4 u_LightPosition[MAX_LIGHT_CNT];
		glm::vec4 u_LightColor[MAX_LIGHT_CNT];
		glm::mat4 u_View{ 0.0f };
		glm::mat4 u_Projection{ 0.0f };
		glm::mat4 u_LightViewMatrix{ 0.0f };
		glm::vec4 u_ViewPosition{ 0.0f };
		int info[MAX_LIGHT_CNT];
	};

	class Shader {
	public:
		virtual ~Shader() = default;
        virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Delete() const = 0;
        // Uniforms
		virtual void BindUBO(const UBO& ubo) = 0;
		virtual void SetUint(const std::string& name, uint32_t value) = 0;
        virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
        virtual void SetFloat(const std::string& name, float value) = 0;
        virtual void SetVec2(const std::string& name, const glm::vec2& value) = 0;
        virtual void SetVec3(const std::string& name, const glm::vec3& value) = 0;
        virtual void SetVec4(const std::string& name, const glm::vec4& value) = 0;
        virtual void SetMat2(const std::string& name, const glm::mat2& mat) = 0;
        virtual void SetMat3(const std::string& name, const glm::mat3& mat) = 0;
        virtual void SetMat4(const std::string& name, const glm::mat4& mat) = 0;
		virtual bool IsCompiled() { return m_Compiled; }
		// For compute shader
		virtual void DispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) const = 0;

		virtual ShaderType GetShaderType() { return m_Type; }
		inline uint32_t GerRendererID() { return m_ShaderID; }

        virtual const std::string& GetName() const = 0;
        static std::shared_ptr<Shader> Create(const std::filesystem::path& filepath);
		static std::shared_ptr<Shader> Create(const std::string& name, const std::string& VertSrc, const std::string& fragSrc);
	protected:
		bool m_Compiled{ false };
		ShaderType m_Type{ ShaderType::None };
		uint32_t m_ShaderID;
	};

	class ShaderLibrary {
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

