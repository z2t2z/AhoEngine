#pragma once

#include "Asset.h"
#include "AssetLoadOptions.h"

namespace Aho {
	class TextureAsset : public Asset {
	public:
		TextureAsset(const std::string& filepath) 
			: Asset(filepath), m_HasMips(false), m_FilpUV(false), m_HDR(false), m_Width(-1), m_Height(-1) {
		}
		TextureAsset(const TextureOptions& opts)
			: Asset(opts.path), m_HasMips(opts.genMipmaps), m_FilpUV(opts.flipUV), m_HDR(false), m_Width(-1), m_Height(-1) {
		}
		bool FilpUV() const { return m_FilpUV; }
		bool IsHDR() const { return m_HDR; }
	private:
		bool m_HasMips;
		bool m_FilpUV;
		bool m_HDR;
		int m_Width;
		int m_Height;
	};
}
