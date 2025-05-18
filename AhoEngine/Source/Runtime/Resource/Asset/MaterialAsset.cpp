#include "Ahopch.h"
#include "MaterialAsset.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Resource/Asset/AssetManager.h"

namespace Aho {
	MaterialAsset::MaterialAsset(const std::string& path, const std::vector<std::pair<TextureUsage, std::string>>& paths)
		: Asset(path) {

		auto assetManager = g_RuntimeGlobalCtx.m_AssetManager;
		auto ecs		  = g_RuntimeGlobalCtx.m_EntityManager;
		
		// Assume texture paths are relative
		std::string rootPath = path;
		auto pos = rootPath.find_last_of("/\\");
		if (pos != std::string::npos) {
			rootPath = rootPath.substr(0, pos);
		}
		for (auto [usage, texPath] : paths) {
			auto textureAsset = assetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions(rootPath + "/" + texPath));
			switch (usage) {
				case (TextureUsage::BaseColor):
					m_Desc.baseColorTex = textureAsset;
					continue;
				case (TextureUsage::Normal):
					m_Desc.normalTex = textureAsset;
					//[[fallthrough]];
					continue;
				case (TextureUsage::Roughness):
					m_Desc.roughnessTex = textureAsset;
					continue;
				case (TextureUsage::Metallic):
					m_Desc.metallicTex = textureAsset;
					continue;
			}
		}
	}
}