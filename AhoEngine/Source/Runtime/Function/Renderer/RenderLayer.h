#pragma once
#include "Renderer.h"
#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Function/Camera/CameraManager.h"
#include "Runtime/Function/Renderer/BufferObject/UBOManager.h"
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
		// TODO: move these to Renderer
		void SetupPrecomputeDiffuseIrradiancePipeline();
	private:
		std::unique_ptr<RenderPass> SetupFXAAPass();
	private:
		void SetupRenderPipeline();
		std::unique_ptr<RenderPass> SetupGenCubemapFromHDRPass();
		std::unique_ptr<RenderPass> SetupPrecomputeIrradiancePass();	// Diffuse 
		std::unique_ptr<RenderPass> SetupPrefilteredPass();				// Specular
		std::unique_ptr<RenderPass> SetupGenLUTPass();
		std::unique_ptr<RenderPass> SetupTransmittanceLUTPass();
		std::unique_ptr<RenderPass> SetupMutiScattLutPass();
		std::unique_ptr<RenderPass> SetupSkyViewLutPass();
	private:
		std::unique_ptr<RenderPass> SetupGBufferPass();
		std::unique_ptr<RenderPass> SetupSSAOPass();
		std::unique_ptr<RenderPass> SetupBlurRPass();
		std::unique_ptr<RenderPass> SetupBlurRGBPass();
		std::unique_ptr<RenderPass> SetupShadowMapPass();
		std::unique_ptr<RenderPass> SetupShadingPass();
		std::unique_ptr<RenderPass> SetupDrawSelectedPass();
		std::unique_ptr<RenderPass> SetupSSRPass();
		std::unique_ptr<RenderPass> SetupHiZPass();
		std::unique_ptr<RenderPass> SetupPostProcessingPass();
		std::unique_ptr<RenderPass> SetupAtmosphericPass();
		//void SetupUBO();
	private:
		EventManager* m_EventManager{ nullptr };
		Renderer* m_Renderer{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	};
};