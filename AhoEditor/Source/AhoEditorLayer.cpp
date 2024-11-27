#include "IamAho.h"
#include "AhoEditorLayer.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"
#include <iomanip>
#include <entt.hpp>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	namespace fs = std::filesystem;

	static ImGuizmo::OPERATION g_Operation = ImGuizmo::OPERATION::NONE;

	enum class ButtonType {
		None = 0,
		Selection,
		Scale,
		Translation,
		Rotation
	};

	static ButtonType g_SelectedButton = ButtonType::None;

	static const float g_ToolBarIconSize = 22.0f;

	AhoEditorLayer::AhoEditorLayer(LevelLayer* levellayer, ResourceLayer* resourceLayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("EditorLayer"), m_ResourceLayer(resourceLayer), m_LevelLayer(levellayer), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	}

	void AhoEditorLayer::OnAttach() {
		AHO_INFO("EditorLayer on attach");
		m_FolderPath = fs::current_path();
		m_AssetPath = m_FolderPath / "Asset";
		m_CurrentPath = m_AssetPath;
		m_FileWatcher.SetCallback(std::bind(&AhoEditorLayer::OnFileChanged, this, std::placeholders::_1));
		m_FileWatcher.AddFileToWatch(m_FolderPath / "ShaderSrc" / "AtmosphericScattering" / "SkyLUT.glsl");			// TODO: resource layer
		m_LightIcon = Texture2D::Create((m_FolderPath / "Asset" / "light-bulb.png").string());	// TODO: resource layer
		m_AddIcon = Texture2D::Create((m_FolderPath / "Asset" / "plusicon.png").string());
		m_CursorIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "cursor.png").string());
		m_TranslationIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "translation.png").string());
		m_RotationIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "rotate.png").string());
		m_ScaleIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "scale.png").string());
		m_BackIcon = Texture2D::Create((m_FolderPath / "Asset" / "Icons" / "back.png").string());
	}

	void AhoEditorLayer::OnDetach() {
	}

	void AhoEditorLayer::OnUpdate(float deltaTime) {
		m_CameraManager->Update(deltaTime, m_IsCursorInViewport);
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
			ImGui::GetStyle().FramePadding.x = padding;

			ImGui::PushFont(io.Fonts->Fonts[0]);
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("Options")) {
					ImGui::MenuItem("DebugView", NULL, &GlobalState::g_ShowDebug);
					ImGui::MenuItem("PickingPass", NULL, &m_PickingPass);
					if (ImGui::BeginMenu("Camera Options")) {
						ImGui::SliderFloat("Mouse Sensitivity", &m_CameraManager->GetSensitivity(), 0.0f, 5.0f);
						ImGui::SliderFloat("Camera Speed", &m_CameraManager->GetSpeed(), 0.0f, 100.0f);
						ImGui::EndMenu();
					}

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
			if (m_IsCursorInViewport && !m_IsClickingEventBlocked && ee->GetMouseButton() == AHO_MOUSE_BUTTON_1) {
				m_ShouldPickObject = true;
			}
			else {
				m_IsClickingEventBlocked = false;
			}
		}
	}

	// TODO: Don't use a polling approach
	bool AhoEditorLayer::OnFileChanged(FileChangedEvent& e) {
		if (e.GetEventType() == EventType::FileChanged) {
			const auto& FileName = e.GetFilePath();
			if (!FileName.empty()) {
				auto newShader = Shader::Create(FileName);
				if (newShader->IsCompiled()) {
					m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(RenderPassType::SkyViewLUT)->SetShader(newShader);
					return true;
				}
			}
		}
		return false;
	}

	void AhoEditorLayer::DrawViewport() {
		ImGuiWindowFlags window_flags = 0
			  | ImGuiWindowFlags_NoDecoration
			  | ImGuiWindowFlags_NoScrollWithMouse
			//| ImGuiWindowFlags_NoDocking			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			//| ImGuiWindowFlags_NoTitleBar
			//| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			//| ImGuiWindowFlags_NoScrollbar
			;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::Begin("Viewport", nullptr, window_flags);
		ImGui::PopStyleVar();

		DrawToolBarOverlay();

		auto [width, height] = ImGui::GetWindowSize();
		m_ViewportWidth = width, m_ViewportHeight = height;
		auto spec = m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::Shading)->GetSpecification();
		if (spec.Width != m_ViewportWidth || spec.Height != m_ViewportHeight/* - ImGui::GetFrameHeight() */) {
			m_Renderer->GetCurrentRenderPipeline()->ResizeRenderTarget(m_ViewportWidth, m_ViewportHeight);
			m_CameraManager->GetMainEditorCamera()->SetProjection(45, width / height, 0.1f, 1000.0f);  // TODO: camera settings
		}

		// TODO: Should be able to select any render result of any passes
		uint32_t RenderResult;
		if (GlobalState::g_ShowDebug) {
			RenderResult = m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::SkyViewLUT)->GetLastColorAttachment();
		}
		else if (m_PickingPass) {
			RenderResult = m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::Atmospheric)->GetLastColorAttachment();
		}
		else {
			RenderResult = m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::FXAA)->GetLastColorAttachment();
		}

		ImGui::Image((ImTextureID)RenderResult, ImGui::GetWindowSize(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		auto [mouseX, mouseY] = ImGui::GetMousePos();
		auto [windowPosX, windowPosY] = ImGui::GetWindowPos();
		int MouseX = mouseX - windowPosX, MouseY = mouseY - windowPosY; // Lower left is[0, 0]
		MouseY = spec.Height - MouseY;
		m_IsViewportFocused = ImGui::IsWindowFocused();
		m_IsCursorInViewport = (MouseX >= 0 && MouseY >= 0 && MouseX < m_ViewportWidth && MouseY < m_ViewportHeight);
		if (m_ShouldPickObject) {
			m_ShouldPickObject = false;
			if (m_IsCursorInViewport) { 
				m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::SSAOGeo)->Bind();
				m_PickPixelData = m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::SSAOGeo)->ReadPixel(TexType::Entity, MouseX, MouseY);
				m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::SSAOGeo)->Unbind();
			}
		}

		DrawGizmo();
		TryGetDragDropTarget();
		DrawLightIcons();
		ImGui::End();
	}

	static ImVec2 s_ImageSize(100.0f, 100.0f);
	static ImVec2 s_Padding(10.0f, 10.0f);
	void AhoEditorLayer::DrawPropertiesPanel() {
		ImGui::Begin(ICON_FA_GEAR " Properties Panel");
		if (!m_Selected) {
			ImGui::End();
			return;
		}
		
		ImGuiIO& io = ImGui::GetIO();
		auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
		auto& Tagc = entityManager->GetComponent<TagComponent>(m_SelectedObject);

		ImFont* boldFont = io.Fonts->Fonts[0];
		ImGui::PushFont(boldFont);
		ImGui::Text(Tagc.Tag.c_str());
		ImGui::PopFont();

		ImGui::Separator();
		ImGui::PushFont(io.Fonts->Fonts[1]);
		bool opened = ImGui::CollapsingHeader("Transform");
		ImGui::PopFont();
		if (opened) {
			ImGui::Separator();
			auto& tc = entityManager->GetComponent<TransformComponent>(m_SelectedObject);
			auto& translation = tc.GetTranslation();
			auto& scale = tc.GetScale();
			auto& rotation = tc.GetRotation();
			DrawVec3Control("Translation", translation);
			DrawVec3Control("Scale", scale, 1.0f);
			DrawVec3Control("Rotation", rotation);
		}

		ImGui::Separator();
		ImGui::PushFont(io.Fonts->Fonts[1]);
		opened = ImGui::CollapsingHeader("Material");
		ImGui::PopFont();

		if (opened) {
			if (entityManager->HasComponent<MaterialComponent>(m_SelectedObject)) {
				auto& materialComp = entityManager->GetComponent<MaterialComponent>(m_SelectedObject);
				if (ImGui::BeginTable("TwoColumnTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
					for (auto& prop : *materialComp.material) {
						switch (prop.m_Type) {
						case TexType::Albedo:
							ImGui::TableNextColumn();
							ImGui::Text("Aledo");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, glm::vec3>) {
									ImGui::ColorPicker3("Color Picker", glm::value_ptr(value));
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Albedo };
								}
							}, prop.m_Value);
							break;
						case TexType::Normal:
							ImGui::TableNextColumn();
							ImGui::Text("Normal");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::Text("Empty");
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Normal };
								}

							}, prop.m_Value);
							break;
						case TexType::Roughness:
							ImGui::TableNextColumn();
							ImGui::Text("Roughness");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::DragFloat("##Roughness", &value, 0.01f, 0.0f, 1.0f);
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Roughness };
								}
							}, prop.m_Value);
							break;
						case TexType::Metallic:
							ImGui::TableNextColumn();
							ImGui::Text("Metallic");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::DragFloat("##Metallic", &value, 0.01f, 0.0f, 1.0f);
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::Metallic };
								}
							}, prop.m_Value);
							break;
						case TexType::AO:
							ImGui::TableNextColumn();
							ImGui::Text("AO");
							ImGui::TableNextColumn();
							std::visit([&](auto& value) {
								using T = std::decay_t<decltype(value)>;
								if constexpr (std::is_same_v<T, std::shared_ptr<Texture2D>>) {
									ImGui::Image((ImTextureID)value->GetTextureID(), s_ImageSize);
								}
								else if constexpr (std::is_same_v<T, float>) {
									ImGui::DragFloat("##AO", &value, 0.01f, 0.0f, 1.0f);
								}
								auto texture = TryGetDragDropTargetTexture();
								if (texture) {
									prop = { texture, TexType::AO };
								}
							}, prop.m_Value);
							break;
						default:
							continue;
						}
					}
					ImGui::EndTable();
				}
				if (ImGui::Button(ICON_FA_FILE_CIRCLE_PLUS " New Slot")) {
					ImGui::OpenPopup("popup_menu_material");
				}
				ImVec2 buttonMin = ImGui::GetItemRectMin(); // upper left
				ImVec2 buttonMax = ImGui::GetItemRectMax(); // lower right
				ImVec2 nxtPos = ImVec2(buttonMin.x, buttonMax.y);
				ImGui::SetNextWindowPos(nxtPos, ImGuiCond_Always);
				if (ImGui::BeginPopup("popup_menu_material")) {
					if (!materialComp.material->HasProperty(TexType::Normal)) {
						if (ImGui::MenuItem("Normal")) {
							materialComp.material->AddMaterialProperties({ 0.0f, TexType::Normal });
						}
					}
					if (!materialComp.material->HasProperty(TexType::Metallic)) {
						if (ImGui::MenuItem("Metallic")) {
							materialComp.material->AddMaterialProperties({ 0.0f, TexType::Metallic });
						}
					}
					if (!materialComp.material->HasProperty(TexType::Roughness)) {
						if (ImGui::MenuItem("Roughness")) {
							materialComp.material->AddMaterialProperties({ 1.0f, TexType::Roughness });
						}
					}
					if (!materialComp.material->HasProperty(TexType::AO)) {
						if (ImGui::MenuItem("AO")) {
							materialComp.material->AddMaterialProperties({ 0.2f, TexType::AO });
						}
					}
					ImGui::EndPopup();
				}
			}
		}

		if (entityManager->HasComponent<PointLightComponent>(m_SelectedObject)) {
			auto& pc = entityManager->GetComponent<PointLightComponent>(m_SelectedObject);
			ImGui::Separator();
			ImGui::Text("Light Color:");
			ImGui::ColorPicker3("Color Picker", glm::value_ptr(pc.color));
		}
		ImGui::End();
	}

	static const float s_LightVisSize = 60.0f;
	void AhoEditorLayer::DrawLightIcons() {
		auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
		const auto& projMat = m_CameraManager->GetMainEditorCamera()->GetProjection();
		const auto& viewMat = m_CameraManager->GetMainEditorCamera()->GetView();
		auto [ww, wh] = ImGui::GetWindowSize();
		auto [wx, wy] = ImGui::GetWindowPos();
		auto view = entityManager->GetView<PointLightComponent, TransformComponent>();
		view.each([&](auto entity, auto& pointLiightComponent, auto& transformComponent) {
			auto clipSpace = projMat * viewMat * glm::vec4(transformComponent.GetTranslation(), 1.0f);
			if (clipSpace.w > 0.0f) {
				auto ndc = clipSpace / clipSpace.w;
				auto ss = ndc * 0.5f + 0.5f;
				if (ss.x >= 0.0f && ss.x <= 1.0f && ss.y >= 0.0f && ss.y <= 1.0f) {
					ImVec2 pos{ wx + ww * ss.x - s_LightVisSize / 2, wy + wh * (1.0f - ss.y) - s_LightVisSize / 2 };
					ImGui::SetCursorScreenPos(pos);
					ImVec2 iconSize{ s_LightVisSize, s_LightVisSize };
					ImGui::Image((ImTextureID)m_LightIcon->GetTextureID(), iconSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
					//DrawCircle(pos, s_LightVisSize);
				}
			}
		});
	}

	void AhoEditorLayer::DrawToolBarOverlay() {
		DrawToolBar();
		DrawManipulationToolBar();
	}

	void AhoEditorLayer::DrawToolBar() {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.1f); // alpha value
		ImGui::SetNextWindowSize(ImVec2{0.0f, 0.0f});
		ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
		ImGui::PopStyleVar();

		ImGuiStyle& style = ImGui::GetStyle();

		if (ImGui::ImageButton("add", (ImTextureID)m_AddIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize, g_ToolBarIconSize })) {
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
			if (ImGui::MenuItem("Cube")) {
				std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(m_ResourceLayer->GetCube(), false);
				m_EventManager->PushBack(e);
			}
			if (ImGui::MenuItem("Sphere")) {
				std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(m_ResourceLayer->GetSphere(), false);
				m_EventManager->PushBack(e);
			}
			if (ImGui::MenuItem("Plane")) {
				std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(m_ResourceLayer->GetPlane(), false);
				m_EventManager->PushBack(e);
			}
			if (ImGui::MenuItem("Cylinder")) {
				std::shared_ptr<PackRenderDataEvent> e = std::make_shared<PackRenderDataEvent>(m_ResourceLayer->GetCylinder(), false);
				m_EventManager->PushBack(e);
			}
			ImGui::EndPopup();
		}
		ImGui::End();
	}

	// TODO: Consider use icon font to avoid aliasing, or implement anti-aliasing first
	void AhoEditorLayer::DrawManipulationToolBar() {
		// Draws a translucent tool bar
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f); 
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.5f, 0.5f, 0.5f, 0.2f));
		ImGui::Begin("ManipulationToolBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		if (ImGui::ImageButton("selectionMode", (ImTextureID)m_CursorIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize },
			ImVec2(0, 0), ImVec2(1, 1),
			g_SelectedButton == ButtonType::Selection ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
			g_SelectedButton = ButtonType::Selection;
			g_Operation = ImGuizmo::OPERATION::NONE;
			m_ShouldPickObject = false;
			m_IsClickingEventBlocked = true;
		}
		ImGui::SameLine();
		auto& io = ImGui::GetIO();
		ImGui::PushFont(io.Fonts->Fonts[2]);
		if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, ImVec2 { g_ToolBarIconSize, g_ToolBarIconSize })) {
				g_SelectedButton = ButtonType::Translation;
				g_Operation = ImGuizmo::OPERATION::TRANSLATE;
				m_ShouldPickObject = false;
				m_IsClickingEventBlocked = true;
		}
		ImGui::PopFont();
		//if (ImGui::ImageButton("translationMode", (ImTextureID)m_TranslationIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize },
		//	ImVec2(0, 0), ImVec2(1, 1),
		//	g_SelectedButton == ButtonType::Translation ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
		//	g_SelectedButton = ButtonType::Translation;
		//	g_Operation = ImGuizmo::OPERATION::TRANSLATE;
		//	m_ShouldPickObject = false;
		//	m_IsClickingEventBlocked = true;
		//}
		ImGui::SameLine();
		if (ImGui::ImageButton("rotationMode", (ImTextureID)m_RotationIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize }, ImVec2(0, 0), ImVec2(1, 1),
			g_SelectedButton == ButtonType::Rotation ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
			g_SelectedButton = ButtonType::Rotation;
			g_Operation = ImGuizmo::OPERATION::ROTATE;
			m_ShouldPickObject = false;
			m_IsClickingEventBlocked = true;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton("scaleMode", (ImTextureID)m_ScaleIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize }, ImVec2(0, 0), ImVec2(1, 1),
			g_SelectedButton == ButtonType::Scale ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
			g_SelectedButton = ButtonType::Scale;
			g_Operation = ImGuizmo::OPERATION::SCALE;
			m_ShouldPickObject = false;
			m_IsClickingEventBlocked = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(4);
		ImGui::End();
	}

	void AhoEditorLayer::DrawGizmo() {
		m_Selected = false;
		if (m_LevelLayer->GetCurrentLevel()) {
			auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
			if (m_PickPixelData && entityManager->IsEntityIDValid(m_PickPixelData)) {
				GlobalState::g_SelectedEntityID = m_PickPixelData;
				GlobalState::g_IsEntityIDValid = true;

				m_SelectedObject = Entity(static_cast<entt::entity>(m_PickPixelData));
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
					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_ViewportWidth, m_ViewportHeight - ImGui::GetFrameHeight());

					// test these: ImGui::IsWindowHovered() && ImGui::IsWindowFocused()
					if (g_Operation != ImGuizmo::OPERATION::NONE) {
						m_IsClickingEventBlocked |= ImGuizmo::IsOver();
						ImGuizmo::Manipulate(glm::value_ptr(viewMat), glm::value_ptr(projMat),
							g_Operation,
							ImGuizmo::MODE::LOCAL,
							glm::value_ptr(transform));
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
					}
				}
			}
			else {
				GlobalState::g_IsEntityIDValid = false;
			}
		}
	}

	void AhoEditorLayer::DrawCircle(const ImVec2& center, float radius) {
		int segments = 32;
		auto draw_list = ImGui::GetWindowDrawList();
		ImU32 color = IM_COL32(255, 255, 0, 255);
		for (int i = 1; i < segments / 2; ++i) {
			float theta = IM_PI * i / (segments / 2);  // 纬线角度
			float r = radius * sin(theta);             // 环的半径
			float y = radius * cos(theta);             // 环的偏移量

			draw_list->AddCircle(ImVec2(center.x, center.y + y), r, color, segments);
		}

		for (int i = 0; i < segments; ++i) {
			float theta = 2 * IM_PI * i / segments;    // 经线角度6
			float x = radius * cos(theta);             // 环的 x 偏移量
			float z = radius * sin(theta);             // 环的 z 偏移量（投影为 y 轴偏移）

			for (int j = 0; j <= segments / 2; ++j) {
				float phi = IM_PI * j / (segments / 2);
				float y = radius * cos(phi);           // 计算垂直方向上的高度
				float r = radius * sin(phi);           // 计算垂直方向上的半径

				ImVec2 point(center.x + x * r / radius, center.y + y);
				if (j > 0) {
					draw_list->PathLineTo(point);
				}
			}
			draw_list->PathStroke(color, ImDrawFlags_None, 1.0f);
		}
	}

	void AhoEditorLayer::DrawMaterialPanel() {
		//if (!)

	}

	void AhoEditorLayer::DrawContentBrowserPanel() {
		ImGui::Begin(ICON_FA_FOLDER " Content Browser");
		if (m_CurrentPath != m_AssetPath) {
			if (ImGui::ImageButton("##back", ImTextureID(m_BackIcon->GetTextureID()), ImVec2{18, 18})) {
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
				std::string identifier = "CONTENT_BROWSER_MESH";
				if (entry.path().extension() != ".obj" && entry.path().extension() != ".fbx") {
					identifier = "CONTENT_BROWSER_TEXTURE";
				}
				const char* itemPayload = relativePath.c_str();
				ImGui::SetDragDropPayload(identifier.c_str(), itemPayload, relativePath.length());
				ImGui::Text("File");
				ImGui::EndDragDropSource();
			}
		}
		ImGui::End();
	}


	static Entity s_SelectedEntity;
	static bool s_IsSelected;
	template<typename T>
	void AhoEditorLayer::DrawNode(const T& node) {
		//ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		//std::string showName = node->bone.name;
		//if (showName.empty()) {
		//	showName = "empty";
		//}
		//if (ImGui::TreeNodeEx(showName.c_str(), flags)) {
		//	if (ImGui::IsItemClicked()) {
		//		s_SelectedEntity = entity;
		//		m_PickPixelData = static_cast<uint32_t>(entity);
		//		GlobalState::g_SelectedEntityID = m_PickPixelData;

		//		for (const auto& child : node->children) {
		//			DrawNode(child);
		//		}
		//	}

		//	ImGui::TreePop();
		//}
	}



	void AhoEditorLayer::DrawSceneHierarchyPanel() {
		ImGui::Begin(ICON_FA_TREE " Hierarchy");
		auto scene = m_LevelLayer->GetCurrentLevel();
		if (!scene) {
			ImGui::End();
			return;
		}
		auto entityManager = scene->GetEntityManager();
		auto view = scene->GetEntityManager()->GetView<TagComponent, TransformComponent>();
		view.each([&](auto entity, TagComponent& tag, TransformComponent& tc) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			if (entityManager->HasComponent<PointLightComponent>(entity) || entityManager->HasComponent<EntityComponent>(entity)) {
				if (ImGui::TreeNodeEx(tag.Tag.c_str(), flags)) {
					if (ImGui::IsItemClicked()) {
						s_SelectedEntity = entity;
						m_PickPixelData = static_cast<uint32_t>(entity);
						GlobalState::g_SelectedEntityID = m_PickPixelData;
					}

					if (entityManager->HasComponent<EntityComponent>(entity)) {
						for (const auto& subEntity : entityManager->GetComponent<EntityComponent>(entity).entities) {
							std::string tag = (entityManager->HasComponent<TagComponent>(Entity(subEntity)) ?
												entityManager->GetComponent<TagComponent>(Entity(subEntity)).Tag : "Untitled");

							if (ImGui::TreeNodeEx(tag.c_str(), flags)) {
								if (ImGui::IsItemClicked()) {
									s_SelectedEntity = subEntity;
									m_PickPixelData = static_cast<uint32_t>(subEntity);
									GlobalState::g_SelectedEntityID = m_PickPixelData;
								}

								// If has skeleton
								if (entityManager->HasComponent<SkeletalComponent>(s_SelectedEntity)) {
									const auto& skeletalComponent = entityManager->GetComponent<SkeletalComponent>(s_SelectedEntity);
									DrawNode(skeletalComponent.root);
								}
								ImGui::TreePop();
							}
						}
					}
					
					// If has skeleton
					if (entityManager->HasComponent<SkeletalComponent>(s_SelectedEntity)) {
						const auto& skeletalComponent = entityManager->GetComponent<SkeletalComponent>(s_SelectedEntity);
						DrawNode(skeletalComponent.root);
					}

					ImGui::TreePop();
				}
			}

		});
		ImGui::End();
	}

	void AhoEditorLayer::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void AhoEditorLayer::TryGetDragDropTarget() {
		bool showPopup = false;
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_MESH");
			if (payload) {
				const char* droppedData = static_cast<const char*>(payload->Data);
				m_DroppedString = std::string(droppedData, payload->DataSize);
				showPopup = true;
				AHO_TRACE("Payload accepted! {}", m_DroppedString);
			}
			ImGui::EndDragDropTarget();
		}
		if (showPopup) {
			ImGui::OpenPopup("Load Settings");
		}
		if (ImGui::BeginPopup("Load Settings")) {
			std::string prompt = "Mesh Type: " + std::string(m_StaticMesh ? "Static Mesh" : "Skeletal Mesh");
			ImGui::Text(prompt.c_str());
			ImGui::Separator();
			ImGui::Checkbox("Static Mesh", &m_StaticMesh);
			ImGui::Separator();
			if (ImGui::Button("Load")) {
				std::shared_ptr<AssetImportedEvent> event = std::make_shared<AssetImportedEvent>(m_DroppedString, !m_StaticMesh);
				AHO_CORE_WARN("Pushing an AssetImportedEvent!");
				m_EventManager->PushBack(event);
				m_DroppedString.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				m_DroppedString.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	std::shared_ptr<Texture2D> AhoEditorLayer::TryGetDragDropTargetTexture() {
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_TEXTURE");
			if (payload) {
				const char* droppedData = static_cast<const char*>(payload->Data);
				m_DroppedString = std::string(droppedData, payload->DataSize);
				AHO_TRACE("Payload accepted! {}", m_DroppedString);

				auto texture = Texture2D::Create(m_DroppedString);
				return texture;
			}
			ImGui::EndDragDropTarget();
		}
		return nullptr;
	}
}