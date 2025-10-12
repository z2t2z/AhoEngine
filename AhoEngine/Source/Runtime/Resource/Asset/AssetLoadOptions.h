#pragma once

#include <glm/glm.hpp>
#include <string>

namespace Aho {
	enum class TextureUsage;
	enum class ShaderFeature : uint32_t;
	enum class ShaderUsage;

	struct AssetLoadOptions {
		std::string path;
		bool watch;
		AssetLoadOptions() = default;
		explicit AssetLoadOptions(const std::string& p, bool watch = false) : path(p), watch(watch) {}
	};

	struct TextureOptions : AssetLoadOptions {
		bool flipUV = false;
		bool genMipmaps = false;
		TextureOptions(const std::string& p, bool flipped = false) : AssetLoadOptions(p), flipUV(flipped) {}
		TextureOptions& setFlipUV(bool f) { flipUV = f; return *this; }
	};

	struct MeshOptions : AssetLoadOptions {
		glm::mat4 PreTransform = glm::mat4(1.0f);
		int LoadFlags = 1 | 2 | 8 | 32 | 262144 | 32768;
		bool IsSkeletalMesh{ false };
		bool BuildBVH{ true };
		MeshOptions(const std::string& p) : AssetLoadOptions(p) {}
		MeshOptions& setPreTransform(const glm::mat4& m) { PreTransform = m; return *this; }
	};

	struct ShaderOptions : AssetLoadOptions {
		ShaderUsage usage;
		ShaderOptions(const std::string& p, ShaderUsage usage) 
			: AssetLoadOptions(p, true), usage(usage) {}
	};

	struct MaterialOptions : AssetLoadOptions {
		std::vector<std::pair<TextureUsage, std::string>> texturePaths;
		MaterialOptions(const std::string& p, const std::vector<std::pair<TextureUsage, std::string>>& texturePaths) 
			: AssetLoadOptions(p, true), texturePaths(texturePaths) {}
	};
}
