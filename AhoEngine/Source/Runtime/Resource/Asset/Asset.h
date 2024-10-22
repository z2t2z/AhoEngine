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
		virtual ~Asset() = default;
		virtual bool Load() = 0;
		//virtual bool Save() = 0;
		bool IsLoaded() { return m_IsLoaded; }
		UUID GetUUID() { return m_UUID; }
	protected:
		UUID m_UUID;
		std::string m_Path;
		bool m_IsLoaded{ false };
	};

	class StaticMesh : public Asset {
	public:
		StaticMesh() = default;
		StaticMesh(const std::string& path) {}
		StaticMesh(const std::vector<std::shared_ptr<MeshInfo>>& SubMesh) : m_SubMesh(SubMesh) {}
		virtual bool Load() override { return false; }
		std::vector<std::shared_ptr<MeshInfo>>::iterator begin() { return m_SubMesh.begin(); }
		std::vector<std::shared_ptr<MeshInfo>>::iterator end() { return m_SubMesh.end(); }
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
	private:
		std::vector<std::shared_ptr<MeshInfo>> m_SubMesh;
	};

	class AnimationAsset : public Asset {
	public:
		AnimationAsset() = default;
		AnimationAsset(const std::string& path) {}
		AnimationAsset(const std::vector<std::shared_ptr<MeshInfo>>& SubMesh) : m_SubMesh(SubMesh) {}
		virtual bool Load() override { return false; }
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
	private:
		std::vector<std::shared_ptr<MeshInfo>> m_SubMesh;
	};

	class SkeletalMesh : public Asset {
	public:
		SkeletalMesh() = default;
		SkeletalMesh(const std::string& path) {}
		SkeletalMesh(const std::vector<std::shared_ptr<SkeletalMeshInfo>>& SubMesh) : m_SubMesh(SubMesh) {}
		virtual bool Load() override { return false; }
		std::vector<std::shared_ptr<SkeletalMeshInfo>>::iterator begin() { return m_SubMesh.begin(); }
		std::vector<std::shared_ptr<SkeletalMeshInfo>>::iterator end() { return m_SubMesh.end(); }
		uint32_t size() { return (uint32_t)m_SubMesh.size(); }
	private:
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
		virtual bool Load() override { return false; }
		MaterialInfo GetMaterialInfo() { return m_MaterialInfo; }
	private:
		MaterialInfo m_MaterialInfo;
	};

}