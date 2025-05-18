#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Math/Math.h"
//#include "Texture.h"

#include <memory>
#include <variant>
#include <glm/glm.hpp>

namespace Aho {
	class Shader;
	class Texture2D;
	enum class TexType;

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

	class MaterialAsset;
	class _Texture;
	// New material system, directly using the shader
	class _Material {
	public:
		_Material() = default;
		~_Material() = default;
		_Material(const std::shared_ptr<MaterialAsset>& matAsset);
		struct MatDescriptor {
			MatDescriptor() = default;
			// --- Base Color ---
			glm::vec3 baseColor			= {1.0f,1.0f,1.0f};
			_Texture* baseColorTex		= nullptr;
			bool useBaseColorTex		= false;

			// -- - Normal Map ---
			_Texture* normalMap			= nullptr;
			bool useNormalMap			= false;

			// --- Metallic (scalar) ---
			float metallic				= 0.0f;
			_Texture* metallicTex		= nullptr;
			bool useMetallicTex			= false;

			// --- Roughness (scalar) ---
			float roughness				= 1.0f;
			_Texture* roughnessTex		= nullptr;
			bool useRoughnessTex		= false;

			// --- Ambient Occlusion (scalar) ---
			float ao					= 0.0f;
			_Texture* aoTex				= nullptr;
			bool useAoTex				= false;

			// --- Emissive Color ---
			glm::vec3 emissive			= {0.0f,0.0f,0.0f};
			_Texture* emissiveTex		= nullptr;
			bool useEmissiveTex			= false;
		};
		void SetBaseColor(const glm::vec3& c) {
			m_Desc.baseColor = c;
			m_Desc.useBaseColorTex = false;
		}
		void SetBaseColor(const std::shared_ptr<_Texture>& tex) {
			m_Desc.baseColorTex = tex.get();
			m_Desc.useBaseColorTex = true;
		}
		void SetNormalMap(const std::shared_ptr<_Texture>& tex) {
			m_Desc.normalMap = tex.get();
			m_Desc.useNormalMap = true;
		}
		void SetMetallic(float m) {
			m_Desc.metallic = m;
			m_Desc.useMetallicTex = false;
		}
		void SetMetallic(const std::shared_ptr<_Texture>& tex) {
			m_Desc.metallicTex = tex.get();
			m_Desc.useMetallicTex = true;
		}
		
		void SetRoughness(float r) {
			m_Desc.roughness = r;
			m_Desc.useRoughnessTex = false;
		}
		void SetRoughness(const std::shared_ptr<_Texture>& tex) {
			m_Desc.roughnessTex = tex.get();
			m_Desc.useRoughnessTex = true;
		}

		void SetAo(float ao) {
			m_Desc.ao = ao;
			m_Desc.useAoTex = false;
		}
		void SetAo(const std::shared_ptr<_Texture>& tex) {
			m_Desc.aoTex = tex.get();
			m_Desc.useAoTex = true;
		}
		static void ApplyToShader(Shader* shader, const _Material& mat, uint32_t& bindingPoint);
		MatDescriptor GetMatDescriptor() const { return m_Desc; }
	private:
		MatDescriptor m_Desc;
	};
}