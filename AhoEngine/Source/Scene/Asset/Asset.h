#pragma once

#include "Scene/UUID.h"

namespace Aho {
	enum class AssetType {
		None = 0,
		Scene,
		Texture,
		Model
	};

	using AssetHandle = UUID;

	class Asset {
	public:
		AssetHandle Handle; // Generate handle

		virtual AssetType GetType() const = 0;
	};

}