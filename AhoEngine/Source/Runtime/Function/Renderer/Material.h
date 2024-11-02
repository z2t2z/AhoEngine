#pragma once

#include "Runtime/Core/Core.h"
#include "Texture.h"
#include "Shader.h"
#include <memory>
#include <glm/glm.hpp>

namespace Aho {
	// Future TODO: different types, translucent

	class Material {
	public:
		Material() {}
		~Material() = default;
		void AddTexture(const std::shared_ptr<Texture2D>& texture) { m_Textures.push_back(texture); m_Outdated = true; }
		template<typename T>
		void SetUniform(const std::string& name, T value);
		void ClearState(const std::shared_ptr<Shader>& shader);
		void Apply(const std::shared_ptr<Shader>& shader, uint32_t texOffset = 0);
		float& GetUniform(const std::string& name) {
			if (m_UniformFloat.contains(name)) {
				return m_UniformFloat[name];
			}
			else {
				AHO_CORE_ERROR("Material does not have this parameter: {}", name);
				float t = 0.0f;
				return t;
			}
		}
		std::vector<std::shared_ptr<Texture2D>>::iterator begin() { return m_Textures.begin(); }
		std::vector<std::shared_ptr<Texture2D>>::iterator end() { return m_Textures.end(); }
	private:
		bool m_Outdated{ true };
		std::vector<std::shared_ptr<Texture2D>> m_Textures;
	private:
		// TODO: use a descriptor or something, this is not intuitive
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