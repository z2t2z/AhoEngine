#pragma once

#include "Runtime/Core/Core.h"
#include "json.hpp"
#include <string>
#include <functional>

namespace Aho {
	using json = nlohmann::json;
	using AssetSerializer = std::function<void(const json&)>;
	using AssetDeserializer = std::function<void(const json&, void*)>;

	static void MeshSerializer(const json& js) {
		
	}

	static std::unordered_map<std::string, AssetSerializer> s_AssetSerializer = {
		{"Mesh", MeshSerializer}
	};

	static std::unordered_map<std::string, AssetDeserializer> s_AssetDeserializer;

	template<typename AssetType>
	static void MeshDeserializer(const json& js, AssetType& assetOut) {

	}
	
	
	class Serializer {
	public:
		template<typename AssetType>
		static void Deserialize(const json& jsonObject, AssetType& assetOut) {
			if (!jsonObject.contains("Type")) {
				AHO_CORE_ERROR("Wrong data format");
				return;
			}

			std::string Type = jsonObject.at("Type");
			if (!s_AssetDeserializer.contains(Type)) {
				AHO_CORE_ERROR("Asset type: {} not supported", Type);
				return;
			}

			s_AssetDeserializer.at(Type)(jsonObject, assetOut);
		}

		template<typename AssetType>
		static bool Write(const AssetType& assetIn) {

		}

	private:

	};
}
