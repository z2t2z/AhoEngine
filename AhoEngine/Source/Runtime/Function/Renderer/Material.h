#pragma once

#include "Runtime/Core/Core.h"
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
		FloatParameter() = default;
		FloatParameter(const std::string& name, float value) : m_Name(name), m_Value(value) {}
		const std::string& GetName() const override { return m_Name; }
		void SetValue(void* val) override { m_Value = *static_cast<float*>(val); }
		void* GetValue() const override { return (void*)&m_Value; }
	private:
		std::string m_Name;
		float m_Value;
	};

	class Vec3Parameter : public MaterialParameter {
	public:
		Vec3Parameter() = default;
		Vec3Parameter(const std::string& name, glm::vec3 value) : m_Name(name), m_Value(value) {}
		const std::string& GetName() const override { return m_Name; }
		void SetValue(void* val) override { m_Value = *static_cast<glm::vec3*>(val); }
		void* GetValue() const override { return (void*)&m_Value; }
	private:
		std::string m_Name;
		glm::vec3 m_Value;
	};

	class Material {
	public:
		Material() {}
		Material(std::string& filepath) { m_Shader = Shader::Create(filepath); }
		~Material() = default;

		void Unbind(const std::shared_ptr<Shader>& shader = nullptr);
		void Apply(const std::shared_ptr<Shader>& shader = nullptr);
		void AddTexture(const std::shared_ptr<Texture2D>& texture) { m_Textures.push_back(texture); }

		std::shared_ptr<Shader> GetShader() const { return m_Shader; }
		void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; }

	private:
		std::vector<std::shared_ptr<Texture2D>> m_Textures;
		std::vector<std::unique_ptr<MaterialParameter>> m_Parameters;
		std::shared_ptr<Shader> m_Shader;
	};

}