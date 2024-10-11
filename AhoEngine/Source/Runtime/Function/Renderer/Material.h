#pragma once

#include "Runtime/Core/Core.h"
#include "Texture.h"
#include "Shader.h"
#include <memory>
#include <glm/glm.hpp>

namespace Aho {
	/* Temporary!! */
	//class MaterialParameter {
	//public:
	//	virtual ~MaterialParameter() = default;
	//	virtual const std::string& GetName() const = 0;
	//	virtual void SetValue(void* value) = 0;
	//	virtual void* GetValue() const = 0;
	//};

	//class FloatParameter : public MaterialParameter {
	//public:
	//	FloatParameter() = default;
	//	FloatParameter(const std::string& name, float value) : m_Name(name), m_Value(value) {}
	//	const std::string& GetName() const override { return m_Name; }
	//	void SetValue(void* val) override { m_Value = *static_cast<float*>(val); }
	//	void* GetValue() const override { return (void*)&m_Value; }
	//private:
	//	std::string m_Name;
	//	float m_Value;
	//};

	//class Vec3Parameter : public MaterialParameter {
	//public:
	//	Vec3Parameter() = default;
	//	Vec3Parameter(const std::string& name, glm::vec3 value) : m_Name(name), m_Value(value) {}
	//	const std::string& GetName() const override { return m_Name; }
	//	void SetValue(void* val) override { m_Value = *static_cast<glm::vec3*>(val); }
	//	void* GetValue() const override { return (void*)&m_Value; }
	//private:
	//	std::string m_Name;
	//	glm::vec3 m_Value;
	//};

	// Future TODO: different types, translucent
	class Material {
	public:
		Material() {}
		~Material() = default;
		Material(std::string& filepath) { m_Shader = Shader::Create(filepath); }
		void UnbindTexture(const std::shared_ptr<Shader>& shader = nullptr);
		void Apply(const std::shared_ptr<Shader>& shader = nullptr);
		void AddTexture(const std::shared_ptr<Texture2D>& texture) { m_Textures.push_back(texture); m_Outdated = true; }
		template<typename T>
		void SetUniform(const std::string& name, T value);
		void SetShader(const std::shared_ptr<Shader>& shader) { m_Shader = shader; m_Outdated = true; }
		std::shared_ptr<Shader> GetShader() const { return m_Shader; }
	private:
		bool m_Outdated{ true };
		std::vector<std::shared_ptr<Texture2D>> m_Textures;
		std::shared_ptr<Shader> m_Shader;

		// TODO: Maybe store pointers directly?
		std::unordered_map<std::string, glm::vec3> m_UniformVec3;
		std::unordered_map<std::string, glm::mat4> m_UniformMat4;
		std::unordered_map<std::string, float> m_UniformFloat;
		std::unordered_map<std::string, int> m_UniformInt;
		std::unordered_map<std::string, uint32_t> m_UniformUint;
	};

	template<>
	inline void Material::SetUniform(const std::string& name, glm::vec3 value) {
		m_UniformVec3[name] = value;
	}
	template<>
	inline void Material::SetUniform(const std::string& name, glm::mat4 value) {
		m_UniformMat4[name] = value;
	}
	template<>
	inline void Material::SetUniform(const std::string& name, float value) {
		m_UniformFloat[name] = value;
	}
	template<>
	inline void Material::SetUniform(const std::string& name, int value) {
		m_UniformInt[name] = value;
	}
	template<>
	inline void Material::SetUniform(const std::string& name, uint32_t value) {
		m_UniformUint[name] = value;
	}
}