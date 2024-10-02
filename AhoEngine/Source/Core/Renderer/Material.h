#pragma once

#include "Core/Core.h"
#include "Texture.h"
#include "Shader.h"
#include <memory>
#include <glm/glm.hpp>

namespace Aho {
	/* Temporary!! */
	class MaterialParameter {
	public:
		virtual ~MaterialParameter() = default;
		virtual const std::string& GetName() const = 0;
		virtual void SetValue(void* value) = 0;
		virtual void* GetValue() const = 0;
	};

	class FloatParameter : public MaterialParameter {
	public:
		FloatParameter(const std::string& name, float value) : m_Name(name), m_Value(value) {}
		const std::string& GetName() const override { return m_Name; }
		void SetValue(void* val) override { m_Value = *static_cast<float*>(val); }
		void* GetValue() const override { return (void*)&m_Value; }
		//template<typename T>
		//T GetValue() const {
		//	return *static_cast<const T*>(GetRawValue());
		//}
	private:
		std::string m_Name;
		float m_Value;
	};

	class Vec3Parameter : public MaterialParameter {
	public:
		Vec3Parameter(const std::string& name, glm::vec3 value) : m_Name(name), m_Value(value) {}
		const std::string& GetName() const override { return m_Name; }
		void SetValue(void* val) override { m_Value = *static_cast<glm::vec3*>(val); }
		void* GetValue() const override { return (void*)&m_Value; }
	private:
		std::string m_Name;
		glm::vec3 m_Value;
	};



	class AHO_API Material {
	public:
		Material() {
			//std::cout << "Material constructed. m_Textures size: " << m_Textures.size() << std::endl;
			AHO_CORE_TRACE("{}", m_Textures.size());
		}

		Material(std::string& filepath) { m_Shader = Shader::Create(filepath); }
		~Material() = default;

		void Apply(const std::shared_ptr<Shader>& shader);
		void AddTexture(std::shared_ptr<Texture2D> texture) { m_Textures.push_back(texture); }
		std::shared_ptr<Shader> GetShader() const { return m_Shader; }
		void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }
	private:
		std::vector<std::shared_ptr<Texture2D>> m_Textures;
		std::vector<std::unique_ptr<MaterialParameter>> m_Parameters;
		std::shared_ptr<Shader> m_Shader;
	};

}