#pragma once

#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"
#include <string>

namespace Aho {
	enum class AssetType {
		None = 0,
		Scene,
		Texture,
		Material,
		Mesh
	};

	std::string_view AssetTypeToString(AssetType type);
	AssetType AssetTypeFromString(std::string_view assetType);
	//using AssetHandle = UUID;


	/*
		Asset can be created through:
		1. Constructed using the data loaded from DCC
		2. Reading .asset file from the disk
	*/

	class Asset {
	public:
		Asset() = default;
		Asset(const std::string& name) : m_Name(name) {}
		virtual ~Asset() = default;
		virtual bool Load() { return true; };
		//virtual bool Save() = 0;
		bool IsLoaded() { return m_IsLoaded; }
		UUID GetUUID() { return m_UUID; }
		std::string GetName() { return m_Name; }
	protected:
		UUID m_UUID;
		std::string m_Name;
		std::string m_Path;
		bool m_IsLoaded{ false };
	};

	class StaticMesh : public Asset {
	public:
		StaticMesh() = default;
		StaticMesh(const std::vector<std::shared_ptr<MeshInfo>>& SubMesh, const std::string& name) : m_SubMesh(SubMesh), Asset(name) {}
		std::vector<std::shared_ptr<MeshInfo>>::iterator begin() { return m_SubMesh.begin(); }
		std::vector<std::shared_ptr<MeshInfo>>::iterator end() { return m_SubMesh.end(); }
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
		const std::vector<std::shared_ptr<MeshInfo>>& GetMeshInfo() { return m_SubMesh; }
	private:
		std::vector<std::shared_ptr<MeshInfo>> m_SubMesh;
	};

	class SkeletalMesh : public Asset {
	public:
		//SkeletalMesh(const std::string& path) {}
		SkeletalMesh(const std::vector<std::shared_ptr<SkeletalMeshInfo>>& SubMesh, 
			const std::map<std::string, BoneNode*>& boneCache, 
			BoneNode* root, 
			const std::string& name, uint32_t cnt)
			: m_SubMesh(SubMesh), m_BoneCache(boneCache), m_Root(root), m_BoneCnt(cnt), Asset(name) {}
		std::vector<std::shared_ptr<SkeletalMeshInfo>>::iterator begin() { return m_SubMesh.begin(); }
		std::vector<std::shared_ptr<SkeletalMeshInfo>>::iterator end() { return m_SubMesh.end(); }
		BoneNode* GetRoot() { return m_Root; }
		std::map<std::string, BoneNode*>& GetBoneCache() { return m_BoneCache; } // read only?
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
		uint32_t GetBoneCnt() { return m_BoneCnt; }
	private:
		uint32_t m_BoneCnt;
		BoneNode* m_Root;
		std::map<std::string, BoneNode*> m_BoneCache;
		std::vector<std::shared_ptr<SkeletalMeshInfo>> m_SubMesh;
	};

	//class ShaderAsset : Asset {
	//public:
	//	ShaderAsset() = default;
	//private:
	//	std::shared_ptr<Shader> m_Shader;
	//};

	class MaterialAsset : public Asset {
	public:
		MaterialAsset() = default;
		MaterialAsset(const std::string& path) {}
		MaterialAsset(const MaterialInfo& materialInfo) : m_MaterialInfo(materialInfo) {}
		MaterialInfo GetMaterialInfo() { return m_MaterialInfo; }
	private:
		MaterialInfo m_MaterialInfo;
	};
}