#pragma once

#include "_Texture.h"
#include "TextureConfig.h"
#include "TextureUsage.h"
#include <string>
#include <memory>

namespace Aho {
	class _Texture;
	class TextureResourceBuilder {
	public:
		TextureResourceBuilder() = default;
		TextureResourceBuilder& Name(const std::string& name)		{ m_Cfg.Label = name; return *this; }
		TextureResourceBuilder& Usage(const TextureUsage usage)		{ m_Cfg.Usage = usage; return *this; }
		TextureResourceBuilder& GenMip(bool gen)					{ m_Cfg.GenMips = gen; return *this; }
		TextureResourceBuilder& Width(uint32_t width)				{ m_Cfg.Width = width; return *this; }
		TextureResourceBuilder& Height(uint32_t height)				{ m_Cfg.Height = height; return *this; }
		TextureResourceBuilder& Dimension(TextureDim dim)			{ m_Cfg.Dim = dim; return *this; }
		TextureResourceBuilder& InternalFormat(InternalFormat fmt)	{ m_Cfg.InternalFmt = fmt; return *this; }
		TextureResourceBuilder& DataFormat(DataFormat fmt)			{ m_Cfg.DataFmt = fmt; return *this; }
		TextureResourceBuilder& DataType(DataType type)				{ m_Cfg.DataType = type; return *this; }
		std::shared_ptr<_Texture> Build() const;
	private:
		TextureConfig m_Cfg;
	};
}
