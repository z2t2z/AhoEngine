#include "IamAho.h"
#include "AhoEditorLayer.h"
#include "entt.hpp"
#include <filesystem>
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	AhoEditorLayer::AhoEditorLayer(LevelLayer* levellayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: m_LevelLayer(levellayer), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void AhoEditorLayer::OnAttach() {
		AHO_INFO("EditorLayer on attach");
		m_FolderPath = std::filesystem::current_path();
		m_CurrentPath = m_FolderPath;
		m_FileWatcher.SetCallback(std::bind(&AhoEditorLayer::OnFileChanged, this, std::placeholders::_1));
		m_FileWatcher.AddFileToWatch(m_FolderPath / "ShaderSrc" / "Shader.glsl");
	}

	void AhoEditorLayer::OnDetach() {
	}

	void AhoEditorLayer::OnUpdate(float deltaTime) {
		m_CameraManager->Update(deltaTime, m_CursorInViewport);
		m_DeltaTime = deltaTime;
		m_FileWatcher.PollFiles();
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
		if (!m_FBO) {
			m_FBO = m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->GetRenderTarget();
		}
		DrawSceneHierarchyPanel();
		DrawContentBrowserPanel();
		DrawViewport();
	}

	void AhoEditorLayer::OnEvent(Event& e) {
		// Handle all input events here
		if (int(e.GetEventType()) & int(EventType::MouseButtonPressed)) {
			e.SetHandled();
			auto ee = (MouseButtonPressedEvent*)&e;
			if (!m_BlockClickingEvent && ee->GetMouseButton() == AHO_MOUSE_BUTTON_1) {
				m_PickObject = true;
			}
		}
	}

	bool AhoEditorLayer::OnFileChanged(FileChangedEvent& e) {
		if (e.GetEventType() == EventType::FileChanged) {
			const auto& FileName = e.GetFilePath();
			if (!FileName.empty()) {
				auto newShader = Shader::Create(FileName);
				if (newShader->IsCompiled()) {
					m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->SetShader(std::move(newShader));
				}
			}
		}
		return true;
	}

	void AhoEditorLayer::DrawViewport() {
		ImGui::Begin("Viewport");
		auto [width, height] = ImGui::GetWindowSize();
		m_FBO->Bind();
		auto spec = m_FBO->GetSpecification();
		if (spec.Width != width || spec.Height != height/* - ImGui::GetFrameHeight() */) {
			m_FBO->Resize(width, height/* - ImGui::GetFrameHeight() */);
			m_CameraManager->GetMainEditorCamera()->SetProjection(45, width / height, 0.1f, 1000.0f);  // TODO: camera settings
		}
		uint32_t RenderResult = m_FBO->GetLastColorAttachment();
		ImGui::Image((void*)(uintptr_t)RenderResult, ImVec2{ width, height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		auto [MouseX, MouseY] = ImGui::GetMousePos();
		auto [windowPosX, windowPosY] = ImGui::GetWindowPos();
		int x = MouseX - windowPosX, y = MouseY - windowPosY - ImGui::GetFrameHeight(); // mouse position in the current window
		y = spec.Height - y;
		if (x >= 0 && y >= 0 && x < width && y < height) {
			m_CursorInViewport = true;
			if (m_PickObject) {
				m_PickObject = false;
				m_PickData = m_FBO->ReadPixel(0, x, y);
			}
		}
		else {
			m_CursorInViewport = false;
		}

		// Object Picking
		m_FBO->Unbind();
		if (m_LevelLayer->GetCurrentLevel()) {
			auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
			if (entityManager->IsEntityIDValid(m_PickData)) {
				m_SelectedObject = Entity(static_cast<entt::entity>(m_PickData));
				if (entityManager->HasComponent<TransformComponent>(m_SelectedObject)) {
					auto& tc = entityManager->GetComponent<TransformComponent>(m_SelectedObject);
					auto& translation = tc.GetTranslation();
					auto& scale = tc.GetScale();
					auto& rotation = tc.GetRotation();
					auto transform = tc.GetTransform();
					// Gizmo
					const auto& viewMat = m_CameraManager->GetMainEditorCamera()->GetView();
					const auto& projMat = m_CameraManager->GetMainEditorCamera()->GetProjection();
					ImGuizmo::SetOrthographic(false);
					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, width, height - ImGui::GetFrameHeight());
					// test these: ImGui::IsWindowHovered() && ImGui::IsWindowFocused()
					m_BlockClickingEvent = ImGuizmo::IsOver();
					ImGuizmo::Manipulate(glm::value_ptr(viewMat), glm::value_ptr(projMat),
						ImGuizmo::OPERATION::TRANSLATE,
						ImGuizmo::MODE::LOCAL,
						glm::value_ptr(transform));
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
					ImGui::Begin("Editor Panel");
					ImGui::Text("%d", m_PickData);
					ImGui::DragFloat3("Translation", glm::value_ptr(translation), 1.0f);
					ImGui::DragFloat3("Scale", glm::value_ptr(scale), 1.0f);
					ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 1.0f);
					ImGui::End();
				}
			}
		}

		// Recieving a drag target
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
			if (payload) {
				const char* droppedData = static_cast<const char*>(payload->Data);
				std::string droppedString(droppedData, payload->DataSize);
				AHO_TRACE("Payload accepted! {}", droppedString);
				std::shared_ptr<AssetImportedEvent> event = std::make_shared<AssetImportedEvent>(droppedString);
				AHO_CORE_WARN("Pushing a AssetImportedEvent!");
				m_EventManager->PushBack(event);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();
	}
	
	namespace fs = std::filesystem;
	void AhoEditorLayer::DrawContentBrowserPanel() {
		ImGui::Begin("Content Browser");
		if (m_CurrentPath != m_FolderPath) {
			if (ImGui::Button("<==")) {
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}
		for (const auto& entry : fs::directory_iterator(m_CurrentPath)) {
			auto fileName = entry.path().filename().string();
			if (ImGui::Button(fileName.c_str())) {
				if (fs::is_directory(entry)) {
					m_CurrentPath = entry.path();
				}
			}
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				auto relativePath = entry.path().string();
				const char* itemPayload = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPayload, relativePath.length());
				ImGui::Text("File");
				ImGui::EndDragDropSource();
			}
		}
		ImGui::End();
	}

	void AhoEditorLayer::DrawSceneHierarchyPanel() {
		ImGui::Begin("Scene Hierarchy");
		//auto [width, height] = ImGui::GetWindowSize();
		auto scene = m_LevelLayer->GetCurrentLevel();
		if (!scene) {
			ImGui::End();
			return;
		}
		//for (const auto& each : scene->GetEntityManager()->GetView<TagComponent>()) {
		//}
		//if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
		//	//m_SelectionContext = {};
		//}
			//Right-click on blank space
		//if (ImGui::BeginPopupContextWindow(nullptr, 1)) {
		//	if (ImGui::MenuItem("Create Empty Entity"))
		//		//m_Context->CreateAObject("Empty Entity");
		//	ImGui::EndPopup();
		//}
		auto view = scene->GetEntityManager()->GetView<TagComponent>();
		view.each([](auto entity, TagComponent& tag) {
			ImGui::Text(tag.Tag.c_str());
		});

		ImGui::End();

		ImGui::Begin("Properties");
		//if (m_SelectionContext) {
		//	DrawComponents(m_SelectionContext);
		//}
		ImGui::End();
	}
}