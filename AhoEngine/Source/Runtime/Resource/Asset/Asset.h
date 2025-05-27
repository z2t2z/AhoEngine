#pragma once

#include "Runtime/Resource/UUID/UUID.h"
#include "Runtime/Resource/ResourceType/ResourceType.h"

#include <string>
#include <filesystem>

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

	class Shader;
	class Texture;

	class Asset {
	public:
		Asset() = default;
		Asset(const std::string& path) : m_Path(path) {
			namespace fs = std::filesystem;
			auto ext = fs::path(path).filename();
			m_Name = ext.string();
			auto pos = m_Name.find(".");
			if (pos != std::string::npos) {
				m_Name = m_Name.substr(0, pos);
			}
		}
		Asset(const std::string& path, const std::string& name) 
			: m_Path(path), m_Name(name) {
		}
		virtual ~Asset() = default;
		virtual bool Load() { return true; };
		void MakeDirty() { m_Dirty = true; }
		void SetName(const std::string& name) { m_Name = name; }
		bool IsDirty() const { return m_Dirty; }
		UUID GetUUID() { return m_UUID; }
		std::string GetName() const { return m_Name; }
		std::string GetPath() const { return m_Path; }
	protected:
		UUID m_UUID;
		std::string m_Name;
		std::string m_Path;
		bool m_Dirty{ true };
	};

	
	class StaticMesh : public Asset {
	public:
		StaticMesh() = default;
		StaticMesh(const std::vector<std::shared_ptr<MeshInfo>>& SubMesh, const std::string& name, uint32_t cnt) 
			: m_SubMesh(SubMesh), m_Count(cnt) {}
		std::vector<std::shared_ptr<MeshInfo>>::iterator begin() { return m_SubMesh.begin(); }
		std::vector<std::shared_ptr<MeshInfo>>::iterator end() { return m_SubMesh.end(); }
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
		uint32_t GetVerticesCount() { return m_Count; }
		const std::vector<std::shared_ptr<MeshInfo>>& GetMeshInfo() { return m_SubMesh; }
	private:
		uint32_t m_Count{ 0 };
		std::vector<std::shared_ptr<MeshInfo>> m_SubMesh;
	};

	class SkeletalMesh : public Asset {
	public:
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

}