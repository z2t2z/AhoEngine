#include "IamAho.h"
#include "AhoEditorLayer.h"
#include <filesystem>
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	AhoEditorLayer::AhoEditorLayer(EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {

	}

	void AhoEditorLayer::OnAttach() {
		AHO_INFO("Editor on attach");
		std::filesystem::path currentPath = std::filesystem::current_path();
		m_FileWatcher.SetCallback(std::bind(&AhoEditorLayer::OnFileChanged, this, std::placeholders::_1));
		m_FileWatcher.AddFileToWatch(currentPath / "ShaderSrc" / "Shader.glsl");
	}

	void AhoEditorLayer::OnDetach() {
	}

	static bool call{ false };

	void AhoEditorLayer::OnUpdate(float deltaTime) {
		m_CameraManager->Update(deltaTime);
		m_DeltaTime = deltaTime;
		m_FileWatcher.PollFiles();
		if (!call) {
			call = true;
			std::filesystem::path path = std::filesystem::current_path() / "Asset" / "sponzaFBX" / "sponza.fbx";
			std::shared_ptr<AssetImportedEvent> event = std::make_shared<AssetImportedEvent>(path.string());
			AHO_CORE_WARN("Pushing a AssetImportedEvent!");
			m_EventManager->PushBack(event);
		}
	}

	void AhoEditorLayer::OnImGuiRender() {
		// Dock space settings
		{
			// Dockspace
			static bool opt_fullscreen = true;
			static bool opt_padding = false;
			static bool open = true;
			bool* p_open = &open;

			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen) {
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}
			else {
				dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			if (!opt_padding)
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", p_open, window_flags);
			if (!opt_padding)
				ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("Options")) {
					// Disabling fullscreen would allow the window to be moved to the front of other windows,
					// which we can't undo at the moment without finer window depth/z control.
					ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
					ImGui::MenuItem("Padding", NULL, &opt_padding);
					ImGui::Separator();

					if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
					if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
					if (ImGui::MenuItem("Flag: NoUndocking", "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
					if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
					if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
					if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
					ImGui::Separator();

					if (ImGui::MenuItem("Exit"))
						Application::Get().ShutDown();

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::End();
		}

		m_Panel->OnImGuiRender();

		// Editor panel
		ImGui::Begin("Editor Panel");
		ImGui::Text("This is the editor panel");
		ImGui::End();

		// Viewport Window
		ImGui::Begin("Viewport");
		auto [width, height] = ImGui::GetWindowSize();
		auto FBO = m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->GetRenderTarget(); // TODO: Make a simpler API, something like GetFinalResult
		FBO->Bind();
		auto spec = FBO->GetSpecification();
		if (spec.Width != width || spec.Height != height) {
			FBO->Resize(width, height);
			m_CameraManager->GetMainEditorCamera()->SetProjection(45, width / height, 0.1f, 1000.0f);
		}
		uint32_t RenderResult = FBO->GetColorAttachmentRendererID(0);
		ImGui::Image((void*)(uintptr_t)RenderResult, ImVec2{ width, height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		FBO->Unbind();
		auto [x, y] = Input::GetMousePosition();
		//auto [w, h] = m_Panel->GetPenalSize();
		//x -= w + 10.0f, y -= 45.0f; // TODO: Think of a better way
		if (x >= 0 && y >= 0 && x < width && y < height) {
			uint32_t readData = FBO->ReadPixel(0, x, y);
		}
		ImGui::End();

		//ImGui::ShowDemoWindow();
	}

	void AhoEditorLayer::OnEvent(Event& e) {
		//EventDispatcher dispatcher(e);
		//AHO_TRACE("Event recieved!");
		//dispatcher.Dispatch<FileChangedEvent>(std::bind(&AhoEditorLayer::OnFileChanged, this, std::placeholders::_1));
	}

	bool AhoEditorLayer::OnFileChanged(FileChangedEvent& e) {
		if (e.GetEventType() == EventType::FileChanged) {
			AHO_TRACE("Event recieved!");
			const auto& FileName = e.GetFilePath();
			if (!FileName.empty()) {
				auto newShader = Shader::Create(FileName);
				if (newShader->IsCompiled()) {
					m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->SetShader(std::move(newShader));
				}
			}
			//e.SetHandled();
			return false;
		}
	}
}