#pragma once

#include <string>
#include <functional>
#include <memory>

namespace Aho {
	class Shader;
	class _Texture;
	class Framebuffer;
	class RenderPassBase;
	enum class TextureUsage;

	struct RenderPassConfig {
		std::string passName;
		std::string shaderPath;
		std::vector<_Texture*> textureAttachments;
		std::function<void(RenderPassBase&)> func;
	};

	class RenderPassBase {
	public:
		using Callback = std::function<void(RenderPassBase&)>;
		RenderPassBase() = default;
		~RenderPassBase();
		void Setup(RenderPassConfig& config);
		void Execute();
		std::string GetPassName() const { return m_Name; }
		void SetCallback(Callback cb) { m_Execute = std::move(cb); }
		Framebuffer* GetRenderTarget() { return m_Framebuffer.get(); }
		void RegisterTextureBuffer(uint32_t textureID, const std::string& bindingName) {
			m_TextureBuffers.emplace_back(textureID, bindingName);
		}
		void BindRegisteredTextureBuffers(uint32_t& slot);
		Shader* GetShader() const { return m_Shader; }
		static uint32_t s_DummyVAO;
	protected:
		std::vector<std::pair<uint32_t, std::string>> m_TextureBuffers;
		std::string m_Name;
		Shader* m_Shader{ nullptr };
		Callback m_Execute;
		std::unique_ptr<Framebuffer> m_Framebuffer;
	};
}