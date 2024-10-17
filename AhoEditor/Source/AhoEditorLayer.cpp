#include "IamAho.h"
#include "AhoEditorLayer.h"
#include "entt.hpp"
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	static ImGuizmo::OPERATION s_Operation = ImGuizmo::OPERATION::TRANSLATE;
	static const float s_ToolBarIconSize = 28.0f;
	AhoEditorLayer::AhoEditorLayer(LevelLayer* levellayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("EditorLayer"), m_LevelLayer(levellayer), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void AhoEditorLayer::OnAttach() {
		AHO_INFO("EditorLayer on attach");
		m_FolderPath = std::filesystem::current_path();
		m_CurrentPath = m_FolderPath;
		m_FileWatcher.SetCallback(std::bind(&AhoEditorLayer::OnFileChanged, this, std::placeholders::_1));
		m_FileWatcher.AddFileToWatch(m_FolderPath / "ShaderSrc" / "pbrShader.glsl");			// TODO: resource layer
		m_LightIcon = Texture2D::Create((m_FolderPath / "Asset" / "light-bulb.png").string());	// TODO: resource layer
		m_PlusIcon = Texture2D::Create((m_FolderPath / "Asset" / "plusicon.png").string());
		m_TranslationIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "translation.png").string());
		m_RotationIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "rotation.png").string());
		m_ScaleIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "scale.png").string());
	}

	void AhoEditorLayer::OnDetach() {

	}

	void AhoEditorLayer::OnUpdate(float deltaTime) {
		m_CameraManager->Update(deltaTime, m_CursorInViewport && m_IsViewportFocused);
		m_DeltaTime = deltaTime;
		m_FileWatcher.PollFiles();
	}

	void AhoEditorLayer::OnImGuiRender() {
		// Dock space settings and some basic settings
		{
			// Dockspace
			static bool opt_fullscreen = true;
			static bool opt_padding = false;
			static bool open = true;
			bool* p_open = &open;

			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_NoTabBar;

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

			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
				ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}
			static bool camSpeed = false;
			static bool sens = false;
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("Options")) {
					ImGui::MenuItem("DebugView", NULL, &m_DrawDepthMap);
					ImGui::MenuItem("PickingPass", NULL, &m_PickingPass);
					if (ImGui::BeginMenu("Camera Options")) {
						ImGui::SliderFloat("Mouse Sensitivity", &m_CameraManager->GetSensitivity(), 0.0f, 5.0f);
						ImGui::SliderFloat("Camera Speed", &m_CameraManager->GetSpeed(), 0.0f, 10.0f);
						ImGui::EndMenu();
					}

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
			// TODO: when setting up renderLayer, fires an event
			m_FBO = m_Renderer->GetCurrentRenderPipeline()->GetResultPass();
		}
		DrawSceneHierarchyPanel();
		DrawContentBrowserPanel();
		DrawPropertiesPanel();
		DrawViewport();
		//ImGui::ShowDemoWindow();
	}

	void AhoEditorLayer::OnEvent(Event& e) {
		// Handle all input events here
		if (e.GetEventType() == EventType::MouseButtonPressed) {
			e.SetHandled();
			auto ee = (MouseButtonPressedEvent*)&e;
			if (m_IsViewportFocused && !m_BlockClickingEvent && ee->GetMouseButton() == AHO_MOUSE_BUTTON_1) {
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
					// Need a proper way!
					//m_Renderer->GetCurrentRenderPipeline()->GetResultPass()->SetShader(newShader);
				}
			}
		}
		return true;
	}

	static bool pressed = false;
	void AhoEditorLayer::DrawViewport() {
		ImGuiWindowFlags window_flags = 0
			//| ImGuiWindowFlags_NoDocking			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			//| ImGuiWindowFlags_NoTitleBar
			//| ImGuiWindowFlags_NoResize
			//| ImGuiWindowFlags_NoMove
			//| ImGuiWindowFlags_NoScrollbar
			//| ImGuiWindowFlags_NoSavedSettings
			;
		ImGui::Begin("Viewport", nullptr, window_flags);
		auto [width, height] = ImGui::GetWindowSize();
		auto fbo = m_Renderer->GetCurrentRenderPipeline()->GetResultPass();
		auto spec = fbo->GetSpecification();
		if (spec.Width != width || spec.Height != height/* - ImGui::GetFrameHeight() */) {
			fbo->Resize(width, height/* - ImGui::GetFrameHeight() */);
			m_Renderer->GetCurrentRenderPipeline()->GetPickingPass()->Resize(0.3 * width, 0.3 * height);
			if (m_DrawDepthMap) {
				m_Renderer->GetCurrentRenderPipeline()->GetDebugPass()->Resize(width, height);
			}
			m_CameraManager->GetMainEditorCamera()->SetProjection(45, width / height, 0.1f, 1000.0f);  // TODO: camera settings
		}

		uint32_t RenderResult;
		if (m_DrawDepthMap) {
			RenderResult = m_Renderer->GetCurrentRenderPipeline()->GetDebugPass()->GetLastColorAttachment();
		}
		else if (m_PickingPass) {
			RenderResult = m_Renderer->GetCurrentRenderPipeline()->GetPickingPass()->GetLastColorAttachment();
		}
		else {
			RenderResult = m_Renderer->GetCurrentRenderPipeline()->GetResultPass()->GetLastColorAttachment();
		}
		ImGui::Image((ImTextureID)RenderResult, ImVec2{ width, height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		auto [MouseX, MouseY] = ImGui::GetMousePos();
		auto [windowPosX, windowPosY] = ImGui::GetWindowPos();
		int x = MouseX - windowPosX, y = MouseY - windowPosY; // mouse position in the current window
		y = spec.Height - y;
		m_IsViewportFocused = ImGui::IsWindowFocused();
		if (m_IsViewportFocused && x >= 0 && y >= 0 && x < width && y < height) {
			m_CursorInViewport = true;
			if (m_PickObject && !m_DrawDepthMap) {
				m_PickObject = false;
				m_Renderer->GetCurrentRenderPipeline()->GetPickingPass()->Bind();
				m_PickData = m_Renderer->GetCurrentRenderPipeline()->GetPickingPass()->ReadPixel(0, 0.3 * x, 0.3 * y);
				m_Renderer->GetCurrentRenderPipeline()->GetPickingPass()->Unbind();
			}
		}
		else {
			m_CursorInViewport = false;
		}

		// Object Picking
		if (m_LevelLayer->GetCurrentLevel()) {
			auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
			if (entityManager->IsEntityIDValid(m_PickData)) {
				m_SelectedObject = Entity(static_cast<entt::entity>(m_PickData));
				if (entityManager->HasComponent<TransformComponent>(m_SelectedObject)) {
					m_Selected = true;
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

					//TODO: use delta matrix here to avoid jittering
					m_BlockClickingEvent = ImGuizmo::IsOver();
					ImGuizmo::Manipulate(glm::value_ptr(viewMat), glm::value_ptr(projMat),
						s_Operation,
						ImGuizmo::MODE::WORLD,
						glm::value_ptr(transform));
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
				}
				else {
					m_BlockClickingEvent = false;
					m_Selected = false;
				} 
			}
			else {
				m_Selected = false;
				m_BlockClickingEvent = false;
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
				AHO_CORE_WARN("Pushing an AssetImportedEvent!");
				m_EventManager->PushBack(event);
			}
			ImGui::EndDragDropTarget();
		}
		DrawLightIcons();

		 //Add button, Play button, Manipulate buttons, etc. Hardcode everything!!!
		{
			auto [ww, wh] = ImGui::GetWindowSize();
			auto [wx, wy] = ImGui::GetWindowPos();
			ImVec2 buttonPos = ImGui::GetCursorScreenPos();
			ImVec2 backup = ImGui::GetCursorPos();
			ImVec2 pos = {wx + 20.0f, wy + ImGui::GetFrameHeight()};
			ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
			ImVec2 OverLaySize{ww, 60.0f};
			ImGui::SetNextWindowSize(OverLaySize, ImGuiCond_Always);
			ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.85f); // alpha value
			if (ImGui::ImageButton("add", (ImTextureID)m_PlusIcon->GetTextureID(), ImVec2{s_ToolBarIconSize ,s_ToolBarIconSize})) {
				ImGui::OpenPopup("popup_menu");
			}
			ImVec2 buttonMin = ImGui::GetItemRectMin(); // upper left
			ImVec2 buttonMax = ImGui::GetItemRectMax(); // lower right
			ImVec2 nxtPos = ImVec2(buttonMin.x, buttonMax.y);
			ImGui::SetNextWindowPos(nxtPos, ImGuiCond_Always);
			if (ImGui::BeginPopup("popup_menu")) {
				if (ImGui::MenuItem("Light")) {
					std::shared_ptr<AddLightSourceEvent> e = std::make_shared<AddLightSourceEvent>(LightType::PointLight);
					m_EventManager->PushBack(e);
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar(); 
			
			ImVec2 IconPos = { wx + 0.85f * ww, wy + ImGui::GetFrameHeight() + 5.0f };
			ImGui::SetCursorScreenPos(IconPos);
			if (ImGui::ImageButton("translationMode", (ImTextureID)m_TranslationIcon->GetTextureID(), ImVec2{ s_ToolBarIconSize ,s_ToolBarIconSize })) {
				s_Operation = ImGuizmo::OPERATION::TRANSLATE;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton("rotationMode", (ImTextureID)m_RotationIcon->GetTextureID(), ImVec2{ s_ToolBarIconSize ,s_ToolBarIconSize })) {
				s_Operation = ImGuizmo::OPERATION::ROTATE;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton("scaleMode", (ImTextureID)m_ScaleIcon->GetTextureID(), ImVec2{ s_ToolBarIconSize ,s_ToolBarIconSize })) {
				s_Operation = ImGuizmo::OPERATION::SCALE;
			}
			ImGui::End(); 
		}
		ImGui::End();
	}

	static float s_ImageSize = 100.0f;
	void AhoEditorLayer::DrawPropertiesPanel() {
		ImGui::Begin("Properties Panel");
		if (!m_Selected) {
			ImGui::End();
			return;
		}
		auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
		auto& tc = entityManager->GetComponent<TransformComponent>(m_SelectedObject);
		auto& translation = tc.GetTranslation();
		auto& scale = tc.GetScale();
		auto& rotation = tc.GetRotation();
		ImGui::DragFloat3("Translation", glm::value_ptr(translation), 0.1f);
		ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f);
		ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.1f); // in degrees

		if (entityManager->HasComponent<MaterialComponent>(m_SelectedObject)) {
			auto& mc = entityManager->GetComponent<MaterialComponent>(m_SelectedObject);
			ImGui::Separator();
			ImGui::Text("Material:");
			ImGui::DragFloat("AO", &mc.material->GetUniform("u_AO"), 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Roughness", &mc.material->GetUniform("u_Roughness"), 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Metalic", &mc.material->GetUniform("u_Metalic"), 0.01f, 0.0f, 1.0f);
			for (const auto& tex : *mc.material) {
				ImGui::Image((ImTextureID)tex->GetTextureID(), ImVec2{s_ImageSize, s_ImageSize}, ImVec2{0, 1}, ImVec2{1, 0});
			}
		}
		if (entityManager->HasComponent<PointLightComponent>(m_SelectedObject)) {
			auto& pc = entityManager->GetComponent<PointLightComponent>(m_SelectedObject);
			ImGui::Separator();
			ImGui::Text("Light Color:");
			ImGui::ColorPicker3("Color Picker", glm::value_ptr(pc.color));
			auto lightData = m_LevelLayer->GetLightData();
			lightData->lightPosition[pc.count] = glm::vec4(translation, 1.0f);
			lightData->lightColor[pc.count] = glm::vec4(pc.color);
		}
		ImGui::End();
	}

	static const float s_LightVisSize = 74.0f;
	void AhoEditorLayer::DrawLightIcons() {
		const auto lightData = m_LevelLayer->GetLightData();
		if (lightData->lightCnt) {
			const auto& proj = m_CameraManager->GetMainEditorCamera()->GetProjection();
			const auto& view = m_CameraManager->GetMainEditorCamera()->GetView();
			auto [ww, wh] = ImGui::GetWindowSize();
			auto [wx, wy] = ImGui::GetWindowPos();
			for (int i = 0; i < lightData->lightCnt; i++) {
				auto p = proj * view * lightData->lightPosition[i];
				if (p.w <= 0.0f) {
					continue;
				}
				auto ndc = p / p.w;
				auto ss = ndc * 0.5f + 0.5f;
				if (ss.x >= 0.0f && ss.x <= 1.0f && ss.y >= 0.0f && ss.y <= 1.0f) {
					ImVec2 pos{ wx + ww * ss.x - s_LightVisSize / 2, wy + wh * (1.0f - ss.y) - s_LightVisSize / 2};
					ImGui::SetCursorScreenPos(pos);
					ImVec2 iconSize{ s_LightVisSize, s_LightVisSize };
					ImGui::Image((ImTextureID)m_LightIcon->GetTextureID(), iconSize, ImVec2{0, 1}, ImVec2{1, 0});
				}
			}
		}
	}

	void AhoEditorLayer::DrawToolBar() {
		ImGuiWindowFlags window_flags = 0
			//| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar
			;
		static bool showMenuBar = false;
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::Begin("toolbar", nullptr, window_flags);
		//ImGui::PopStyleVar();
		if (ImGui::IsWindowDocked()) {
			auto* wnd = ImGui::FindWindowByName("toolbar");
			if (wnd) {
				ImGuiDockNode* node = wnd->DockNode;
				if (node) {
					node->TabBar->Flags |= ImGuiDockNodeFlags_NoTabBar;
					//node->WantHiddenTabBarToggle = true;
				}
			}
		}
		if (ImGui::ImageButton("Add", (ImTextureID)m_PlusIcon->GetTextureID(), ImVec2{ s_ToolBarIconSize ,s_ToolBarIconSize })) {
			//showMenuBar = true;
			//ImGui::MenuItem("add", NULL, &showMenuBar);
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

		//ImGui::Begin("Light Properties");
		//auto lightData = m_LevelLayer->GetLightData();
		//for (int i = 0; i < 1; i++) {
		//	ImGui::Text("Light %d", i + 1);
		//	std::string lightPos = "Light Position ";// +std::to_string(i + 1);
		//	std::string lightColor = "Light Color ";// +std::to_string(i + 1);
		//	//ImGui::DragFloat3(lightPos.c_str(), glm::value_ptr(lightData->lightPosition[i]), 0.01f);
		//	//ImGui::DragFloat3(lightColor.c_str(), glm::value_ptr(lightData->lightColor[i]), 0.01f);
		//	auto& pos = m_LevelLayer->GetUBO()->u_LightPosition[0];
		//	ImGui::DragFloat3(lightPos.c_str(), glm::value_ptr(pos), 0.01f);
		//	ImGui::DragFloat3(lightColor.c_str(), glm::value_ptr(m_LevelLayer->GetUBO()->u_LightColor[0]), 0.01f, 0.0f, 1.0f);
		//	ImGui::Separator();
		//}
		//ImGui::End();
	}
}