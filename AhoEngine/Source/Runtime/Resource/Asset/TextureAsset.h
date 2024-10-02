#pragma once

#include "Asset.h"
#include "Runtime/Function/Renderer/Texture.h"

#include "json.hpp"

namespace Aho {
	//class TextureAsset : public Asset {
	//public:
	//	TextureAsset(const std::string& filepath, std::shared_ptr<Texture2D> _Texture) : Asset(filepath), m_Texture(_Texture) {}
	//	std::shared_ptr<Texture2D> GetTexture() const { return m_Texture; }
	//	void SetTexture(std::shared_ptr<Texture2D> texture)	{ m_Texture = texture; }
	//	void SetTexture(std::shared_ptr<Texture2D>&& texture) { m_Texture = std::move(texture); }
	//private:
	//	std::shared_ptr<Texture2D> m_Texture;
	//};
}
