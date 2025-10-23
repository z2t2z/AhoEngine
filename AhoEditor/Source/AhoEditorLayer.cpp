#include "AhoEditorLayer.h"
#include "FileExplorer.h"
#include "EditorUI/EditorGlobalContext.h"
#include "Runtime/Core/Events/MainThreadDispatcher.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"

#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <entt.hpp>
#include <filesystem>

namespace Aho {
	namespace fs = std::filesystem;
	static ImGuizmo::OPERATION g_Operation = ImGuizmo::OPERATION::NONE;

	AhoEditorLayer::AhoEditorLayer(LevelLayer* levellayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("EditorLayer"), m_LevelLayer(levellayer), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void AhoEditorLayer::OnAttach() {
		m_ContentBrowser.Initialize();
		m_HierachicalPanel.Initialize();
		m_Viewport.Initialize(m_Renderer, m_LevelLayer, m_EventManager, m_CameraManager->GetMainEditorCamera());

		m_EditorCamEntity = g_RuntimeGlobalCtx.m_EntityManager->CreateEntity();
		g_RuntimeGlobalCtx.m_EntityManager->AddComponent<EditorCamaraComponent>(m_EditorCamEntity, m_CameraManager->GetMainEditorCamera());
	}

	void AhoEditorLayer::OnDetach() {
	}

	void AhoEditorLayer::OnUpdate(float deltaTime) {
		MainThreadDispatcher::Get().Execute();
		// Update editer camera
		if (m_Viewport.IsCursorInViewport() && Input::IsMouseButtonPressed(AHO_MOUSE_BUTTON_RIGHT)) {
			bool firstClick = false;
			if (!m_CursorLocked) {
				firstClick = true;
				m_CursorLocked = true;
				Input::LockCursor();
			}
			bool changed = m_CameraManager->Update(deltaTime, firstClick);
			if (changed) {
				auto& cmp = g_RuntimeGlobalCtx.m_EntityManager->GetComponent<EditorCamaraComponent>(m_EditorCamEntity);
				cmp.Dirty = true;
			}
		} else if (m_CursorLocked) {
			m_CursorLocked = false;
			Input::UnlockCursor();
		}

		m_DeltaTime = deltaTime;
	}

	void AhoEditorLayer::OnImGuiRender() {
		// Dock space settings and some basic settings
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
			} else {
				dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			if (!opt_padding)
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			ImGuiIO& io = ImGui::GetIO();
			ImFont* titleFont = io.Fonts->Fonts[2];
			ImGui::PushFont(titleFont);
			ImGui::GetStyle().TabRounding = 15.0f;
			float padding = ImGui::GetStyle().FramePadding.x;
			ImGui::GetStyle().FramePadding.x = 10.0f;
			ImGui::Begin("DockSpace Demo", p_open, window_flags);
			if (!opt_padding)
				ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}
			static bool camSpeed = false;
			static bool sens = false;

			static bool showDemo = false;
			static bool showRenderSettings = false;
			static bool showFrameTimePenal = false;

			ImGui::GetStyle().FramePadding.x = padding;
			ImGui::PushFont(io.Fonts->Fonts[0]);
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("Options")) {
					if (ImGui::BeginMenu("Camera Options")) {
						ImGui::SliderFloat("Mouse Sensitivity", &m_CameraManager->GetSensitivity(), 0.0f, 5.0f);
						ImGui::SliderFloat("Camera Speed", &m_CameraManager->GetSpeed(), 0.0f, 100.0f);
						ImGui::EndMenu();
					}

					ImGui::MenuItem("ShowImGuiDemoWindow", NULL, &showDemo);
					ImGui::MenuItem("Render Settings", NULL, &showRenderSettings);
					ImGui::MenuItem("Frame Time", NULL, &showFrameTimePenal);

					ImGui::Separator();

					if (ImGui::MenuItem(ICON_FA_POWER_OFF " Exit"))
						Application::Get().ShutDown();

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ImGui::PopFont();
			ImGui::PopFont();
			ImGui::End();
			ImGui::GetStyle().TabRounding = 0.0f;

			if (showDemo) {
				ImGui::ShowDemoWindow();
			}
			if (showRenderSettings) {
				m_RenderSettingsPenal.Draw();
			}
			if (showFrameTimePenal) {
				m_DbgPenal.Draw();
			}
		}

		m_ContentBrowser.Draw();
		m_Viewport.Draw();
		m_HierachicalPanel.Draw();
		m_PropertiesPanel.Draw();
	}

	void AhoEditorLayer::OnEvent(Event& e) {
	}
}