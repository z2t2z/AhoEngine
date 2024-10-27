#pragma once
#include "Renderer.h"
#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include <memory>

namespace Aho {
	class RenderLayer : public Layer {
	public:
		RenderLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager);
		~RenderLayer() { delete m_Renderer; }
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
		void SetRenderer(Renderer* renderer) { m_Renderer = renderer; }
		Renderer* GetRenderer() { return m_Renderer; }
	private:
		void SetupForwardRenderPipeline();
		RenderPass* SetupGBufferPass();
		RenderPass* SetupSSAOPass();
		RenderPass* SetupSSAOBlurPass();
		RenderPass* SetupSSAOLightingPass();
		RenderPass* SetupDebugPass();
		RenderPass* SetupShadowMapPass();
		RenderPass* SetupMainPass();
		RenderPass* SetupPickingPass();
		RenderPass* SetupSSRPass();
		RenderPass* SetupHiZPass();
		RenderPass* SetupDrawLinePass();
		RenderPass* SetupPostProcessingPass();
	private:
		EventManager* m_EventManager{ nullptr };
		Renderer* m_Renderer{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	};
};