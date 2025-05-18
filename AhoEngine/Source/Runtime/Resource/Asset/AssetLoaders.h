#pragma once

#include <vector>

namespace Aho {
	struct MeshOptions;
	struct Mesh;
	enum class TextureUsage;
	// Used when loading only
	struct MaterialPaths {
		MaterialPaths() = default;
		MaterialPaths(const std::vector<std::pair<TextureUsage, std::string>>& ups)
			: UsagePaths(ups) {
		}
		std::vector<std::pair<TextureUsage, std::string>> UsagePaths;
	};

	class AssetLoader {
	public:
		static bool MeshLoader(const MeshOptions& options, std::vector<Mesh>& mesh, std::vector<MaterialPaths>& mat);
	};
}
