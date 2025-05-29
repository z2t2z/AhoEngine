#pragma once

#include "DisneyPrincipled.h"
#include "Runtime/Core/Core.h"
#include "Runtime/Core/Math/Math.h"

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

	// Delete this class when the new material system is stable
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
	struct MaterialTextureDescriptor {
		MaterialTextureDescriptor() = default;
		// --- Base Color ---
		_Texture* baseColorTex = nullptr;
		bool useBaseColorTex = false;

		// -- - Normal Map ---
		_Texture* normalMap = nullptr;
		bool useNormalMap = false;

		// --- Metallic (scalar) ---
		_Texture* metallicTex = nullptr;
		bool useMetallicTex = false;

		// --- Roughness (scalar) ---
		_Texture* roughnessTex = nullptr;
		bool useRoughnessTex = false;

		// --- Ambient Occlusion (scalar) ---
		_Texture* aoTex = nullptr;
		bool useAoTex = false;

		// --- Emissive Color ---
		_Texture* emissiveTex = nullptr;
		bool useEmissiveTex = false;
	};
	// New material system, directly using the shader
	class _Material {
	public:
		_Material() = default;
		~_Material() = default;
		_Material(const std::shared_ptr<MaterialAsset>& matAsset);
		void SetBaseColor(const glm::vec3& c) {
			m_ParamDesc.baseColor = c;
			m_TextureDesc.useBaseColorTex = false;
		}
		void SetBaseColor(const std::shared_ptr<_Texture>& tex) {
			m_TextureDesc.baseColorTex = tex.get();
			m_TextureDesc.useBaseColorTex = true;
		}
		void SetNormalMap(const std::shared_ptr<_Texture>& tex) {
			m_TextureDesc.normalMap = tex.get();
			m_TextureDesc.useNormalMap = true;
		}
		void SetMetallic(float m) {
			m_ParamDesc.metallic = m;
			m_TextureDesc.useMetallicTex = false;
		}
		void SetMetallic(const std::shared_ptr<_Texture>& tex) {
			m_TextureDesc.metallicTex = tex.get();
			m_TextureDesc.useMetallicTex = true;
		}
		
		void SetRoughness(float r) {
			m_ParamDesc.roughness = r;
			m_TextureDesc.useRoughnessTex = false;
		}
		void SetRoughness(const std::shared_ptr<_Texture>& tex) {
			m_TextureDesc.roughnessTex = tex.get();
			m_TextureDesc.useRoughnessTex = true;
		}

		void SetAo(float ao) {
			m_ParamDesc.ao = ao;
			m_TextureDesc.useAoTex = false;
		}
		void SetAo(const std::shared_ptr<_Texture>& tex) {
			m_TextureDesc.aoTex = tex.get();
			m_TextureDesc.useAoTex = true;
		}
		void ApplyToShader(Shader* shader, uint32_t& bindingPoint) const;
		MaterialDescriptor& GetMatDescriptor() { return m_ParamDesc; }
		MaterialTextureDescriptor& GetMatTextureDescriptor() { return m_TextureDesc; }
	private:
		MaterialDescriptor m_ParamDesc;
		MaterialTextureDescriptor m_TextureDesc;
	};
}