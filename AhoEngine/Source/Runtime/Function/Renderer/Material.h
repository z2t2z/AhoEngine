#pragma once

#include "Runtime/Core/Core.h"
#include "Texture.h"
#include "Shader.h"
#include <memory>
#include <variant>
#include <glm/glm.hpp>

namespace Aho {
	enum MaterialMaskEnum {
		Empty		 = 0,
		AlbedoMap	 = 1 << 0,
		NormalMap	 = 1 << 1,
		RoughnessMap = 1 << 2,
		MetallicMap  = 1 << 3,
		AOMap		 = 1 << 4,
		All          = AlbedoMap | NormalMap | RoughnessMap | MetallicMap | AOMap
	};

	enum BxDFFlags {
		Unset = 0,
		Reflection = 1 << 0,
		Transmission = 1 << 1,
		Diffuse = 1 << 2,
		Glossy = 1 << 3,
		Specular = 1 << 4,
		DiffuseReflection = Diffuse | Reflection,
		DiffuseTransmission = Diffuse | Transmission,
		GlossyReflection = Glossy | Reflection,
		GlossyTransmission = Glossy | Transmission,
		SpecularReflection = Specular | Reflection,
		SpecularTransmission = Specular | Transmission,
		BxDFAll = Diffuse | Glossy | Specular | Reflection | Transmission
	};

	inline MaterialMaskEnum operator|(MaterialMaskEnum a, MaterialMaskEnum b) {
		return static_cast<MaterialMaskEnum>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline MaterialMaskEnum operator&(MaterialMaskEnum a, MaterialMaskEnum b) {
		return static_cast<MaterialMaskEnum>(static_cast<int>(a) & static_cast<int>(b));
	}

	inline MaterialMaskEnum operator^(MaterialMaskEnum a, MaterialMaskEnum b) {
		return static_cast<MaterialMaskEnum>(static_cast<int>(a) ^ static_cast<int>(b));
	}

	inline MaterialMaskEnum operator~(MaterialMaskEnum a) {
		return static_cast<MaterialMaskEnum>(~static_cast<int>(a));
	}

	inline MaterialMaskEnum& operator|=(MaterialMaskEnum& a, MaterialMaskEnum b) {
		a = a | b;
		return a;
	}

	inline MaterialMaskEnum& operator&=(MaterialMaskEnum& a, MaterialMaskEnum b) {
		a = a & b;
		return a;
	}

	inline MaterialMaskEnum& operator^=(MaterialMaskEnum& a, MaterialMaskEnum b) {
		a = a ^ b;
		return a;
	}
	
	struct MaterialProperty {
		using Value = std::variant<std::shared_ptr<Texture2D>, glm::vec3, float>;
		Value m_Value;
		TexType m_Type;
		MaterialProperty(const std::shared_ptr<Texture2D>& tex, TexType type) : m_Value(tex), m_Type(type) {}
		MaterialProperty(const glm::vec3& tex, TexType type) : m_Value(tex), m_Type(type) {}
		MaterialProperty(float tex, TexType type) : m_Value(tex), m_Type(type) {}
	};

	class Material {
	public:
		Material() {}
		~Material() = default;
		void AddMaterialProperties(const MaterialProperty& prop) { m_Properties.push_back(prop); }
		void Apply(const std::shared_ptr<Shader>& shader, uint32_t& texOffset);
		const MaterialProperty& GetProperty(TexType type);
		std::vector<MaterialProperty>::iterator begin() { return m_Properties.begin(); }
		std::vector<MaterialProperty>::iterator end() { return m_Properties.end(); }
		size_t GetPropCount() { return m_Properties.size(); }
		MaterialProperty& GetProperty(size_t index) { return m_Properties[index]; }
		bool HasProperty(TexType type);
	private:
		void ClearState(const std::shared_ptr<Shader>& shader);
		std::vector<MaterialProperty> m_Properties;
	};
}