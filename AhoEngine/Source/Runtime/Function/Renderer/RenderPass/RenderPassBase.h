#pragma once

#include <string>
#include <functional>
#include <memory>

namespace Aho {
	class Shader;
	class _Texture;
	class Framebuffer;
	class RenderPassBase;
	enum class ShaderUsage;

	struct RenderPassConfig {
		std::string passName;
		std::string shaderPath;
		ShaderUsage usage;
		std::vector<_Texture*> attachmentTargets;
		std::vector<std::pair<std::string, _Texture*>> inputTextures;
		std::function<void(RenderPassBase&)> func;
	};

	class RenderPassBase {
	public:
		using Callback = std::function<void(RenderPassBase&)>;
		RenderPassBase() = default;
		RenderPassBase(const RenderPassConfig& config) {
			Setup(config);
		}
		~RenderPassBase();
		void Setup(RenderPassConfig config);
		void Execute();
		bool Resize(uint32_t width, uint32_t height) const;
		std::string GetPassName() const { return m_Name; }
		float GetFrameTime() const { return m_FrameTime; }
		void SetCallback(Callback cb) { m_Execute = std::move(cb); }
		Framebuffer* GetRenderTarget() { return m_Framebuffer.get(); }
		void RegisterTextureBuffer(_Texture* texture, const std::string& bindingName) {
			m_InputTextures.emplace_back(bindingName, texture);
		}
		_Texture* GetTextureAttachmentByIndex(size_t index) {
			AHO_CORE_ASSERT(index < m_Attachments.size());
			return m_Attachments.at(index);
		}
		void BindRegisteredTextureBuffers(uint32_t& slot) const;
		Shader* GetShader() const { return m_Shader; }
		static uint32_t s_DummyVAO;
	protected:
		float m_FrameTime{ 0.0f };
		std::vector<std::pair<std::string, _Texture*>> m_InputTextures; // Uniforms 
		std::string m_Name;
		Shader* m_Shader{ nullptr };
		Callback m_Execute;
		std::unique_ptr<Framebuffer> m_Framebuffer;
		std::vector<_Texture*> m_Attachments; // Has a copy in fbo too, used for compute shader(does not need fbo)
	};
}