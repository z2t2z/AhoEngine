#include "Ahopch.h"
#include "ResourceManager.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Resource/Asset/MaterialAsset.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Resource/Asset/Mesh/MeshAsset.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Level/EcS/Entity.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/VertexArray.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"

namespace Aho {
	// Load material here


	std::shared_ptr<_Texture> ResourceManager::LoadGPUTexture(const std::shared_ptr<TextureAsset>& textureAsset) {
		if (m_TextureCached.count(textureAsset)) {
			return m_TextureCached.at(textureAsset);
		}

		std::shared_ptr<_Texture> texture = std::make_shared<_Texture>(textureAsset);
		m_TextureCached[textureAsset] = texture;
		return texture;
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

	Entity ResourceManager::CreateGameObject(const std::string& name) {
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
