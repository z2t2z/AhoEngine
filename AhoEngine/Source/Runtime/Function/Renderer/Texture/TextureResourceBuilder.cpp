#include "Ahopch.h"
#include "TextureResourceBuilder.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Function/Renderer/Shader/ShaderVariantManager.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"

namespace Aho {
	std::shared_ptr<_Texture> TextureResourceBuilder::Build() const {
		std::shared_ptr<_Texture> buffer =  std::make_shared<_Texture>(m_Cfg);
		auto resManager = g_RuntimeGlobalCtx.m_Resourcemanager;
		resManager->RegisterBufferTexture(m_Cfg.Label, buffer);
		return buffer;
	}
}