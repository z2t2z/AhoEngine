#include "Ahopch.h"
#include "ResourceManager.h"
#include "Runtime/Core/Events/EngineEvents.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/Asset/Mesh/MeshAsset.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/VertexArray.h"
#include "Runtime/Function/Renderer/Shader/ShaderVariantManager.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"

namespace Aho {
	void ResourceManager::Initialize() {
		m_ShaderVariantManager = std::make_unique<ShaderVariantManager>();
		m_ShaderVariantManager->Initialize();
	}

	// Load material here
	std::shared_ptr<_Texture> ResourceManager::LoadGPUTexture(const std::shared_ptr<TextureAsset>& textureAsset) {
		if (m_TextureCached.count(textureAsset)) {
			return m_TextureCached.at(textureAsset);
		}

		std::shared_ptr<_Texture> texture = std::make_shared<_Texture>(textureAsset);
		m_TextureCached[textureAsset] = texture;
		return texture;
	}

	void ResourceManager::RegisterBufferTexture(const std::string& name, const std::shared_ptr<_Texture>& buffer) {
		AHO_CORE_ASSERT(!m_BufferTextures.count(name));
		m_BufferTextures[name] = buffer;
	}

	std::shared_ptr<_Texture> ResourceManager::GetBufferTexture(const std::string& name) const {
		AHO_CORE_ASSERT(m_BufferTextures.count(name));
		return m_BufferTextures.at(name);
	}

	std::shared_ptr<VertexArray> ResourceManager::LoadVAO(const std::shared_ptr<MeshAsset>& meshAsset) {
		if (m_VAOCached.count(meshAsset)) {
			return m_VAOCached.at(meshAsset);
		}

		std::shared_ptr<VertexArray> vao;
		vao.reset(VertexArray::Create());
		vao->Init(meshAsset->GetMesh());

		m_VAOCached[meshAsset] = vao;
		return vao;
	}

	std::shared_ptr<Shader> ResourceManager::LoadShaderResource(const std::shared_ptr<ShaderAsset>& shaderAsset, ShaderFeature feature) {
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		auto shaderEntity = ecs->CreateEntity();
		std::shared_ptr<Shader> shader{ nullptr };
		AHO_CORE_ASSERT(m_ShaderVariantManager->LoadVariant(shader, shaderAsset, feature), shaderAsset->GetPath());
		ecs->AddComponent<ShaderResourceComponent>(shaderEntity, shaderAsset, shader, shaderAsset->GetUsage());
		return shader;
	}

	Entity ResourceManager::LoadIBL(std::shared_ptr<TextureAsset>& textureAsset) {
		auto texture = LoadGPUTexture(textureAsset);
		auto entity = CreateGameObject("GO_" + textureAsset->GetName());
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		auto& iblComp = ecs->AddComponent<IBLComponent>(entity);
		iblComp.IBL = std::make_unique<IBL>(texture.get());
		iblComp.EnvTexture = texture.get();
		//auto renderer = g_RuntimeGlobalCtx.m_Renderer;
		return entity;
	}

	Entity ResourceManager::CreateGameObject(const std::string& name) {
		// TODO
		std::string s = name;
		if (m_GameObjects.count(name)) {
			int& num = m_GameObjects.at(name);
			s = name + std::to_string(num++);
		}
		else {
			m_GameObjects[name] = 1;
		}
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		auto go = ecs->CreateEntity();
		ecs->AddComponent<GameObjectComponent>(go, s);
		return go;
	}
}
