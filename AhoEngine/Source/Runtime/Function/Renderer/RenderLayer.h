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
		UBO* GetUBO() { return &m_UBO; }
	private:
		void SetupForwardRenderPipeline();
		RenderPass* SetupSSAOGeoPass();
		RenderPass* SetupSSAOPass();
		RenderPass* SetupDebugPass(const std::shared_ptr<Framebuffer>& fbo);
		RenderPass* SetupDepthPass();
		RenderPass* SetupMainPass();
		RenderPass* SetupPickingPass();
	private:
		UBO m_UBO;
		EventManager* m_EventManager{ nullptr };
		Renderer* m_Renderer{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	};
};