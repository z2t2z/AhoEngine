#include "Ahopch.h"
#include "RenderLayer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Function/Renderer/Shader.h"
#include "Runtime/Platform/OpenGL/OpenGLShader.h"
#include "Runtime/Platform/OpenGL/OpenGLTexture.h"
#include "Runtime/Platform/OpenGL/OpenGLFramebuffer.h"
#include <imgui.h>

namespace Aho {
	static std::filesystem::path g_CurrentPath;

	static glm::mat4 g_Projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

	static glm::mat4 g_Views[] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	RenderLayer::RenderLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("RenderLayer"), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void RenderLayer::OnAttach() {
		AHO_CORE_INFO("RenderLayer on attach");
		//SetupUBO();

		//g_CurrentPath = std::filesystem::current_path();
		//Texture* m_HDR = new OpenGLTexture2D((g_CurrentPath / "Asset" / "HDR" / "rogland_clear_night_4k.hdr").string()/*, true*/);
		//m_HDR->SetTexType(TexType::HDR);

		//DeferredShadingPipeline* dpl = new DeferredShadingPipeline();
		//m_Renderer->AddRenderPipeline();

		//SetupPrecomputeDiffuseIrradiancePipeline();
		//m_Renderer->GetPipeline(RenderPipelineType::RPL_Precompute)->Execute();
		//SetupRenderPipeline();
	}

	void RenderLayer::OnDetach() {
	}

	void RenderLayer::OnUpdate(float deltaTime) {
		m_Renderer->Render();
	}

	void RenderLayer::OnImGuiRender() {
	}

	void RenderLayer::OnEvent(Event& e) {
		if (e.GetEventType() == EventType::UploadRenderData) {
			e.SetHandled();
			AHO_CORE_WARN("Recieving a UploadRenderDataEvent!");
			for (const auto& data : ((UploadRenderDataEvent*)&e)->GetRawData()) {
				m_Renderer->AddRenderData(data);
			}
		}
	}
}