#include "Viewport.h"
#include "FileExplorer.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

	Viewport::Viewport() {
	}

	void Viewport::Initialize(Renderer* renderer, LevelLayer* levelLayer, ResourceLayer* resourceLayer, EventManager* manager, const std::shared_ptr<Camera>& camera) {
		m_Renderer = renderer;
		m_LevelLayer = levelLayer;
		m_ResourceLayer = resourceLayer;
		m_EventManager = manager;
		m_EditorCamera = camera;

		auto path = fs::current_path();
		m_LightIcon = Texture2D::Create((path / "Asset" / "light-bulb.png").string());
		m_AddIcon = Texture2D::Create((path / "Asset" / "plusicon.png").string());
		m_CursorIcon = Texture2D::Create((path / "Asset" / "Icons" / "cursor.png").string());
		m_TranslationIcon = Texture2D::Create((path / "Asset" / "Icons" / "translation.png").string());
		m_RotationIcon = Texture2D::Create((path / "Asset" / "Icons" / "rotate.png").string());
		m_ScaleIcon = Texture2D::Create((path / "Asset" / "Icons" / "scale.png").string());
	}
	
	void Viewport::Draw() {
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
		if (m_Renderer->OnViewportResize(width, height)) {
			m_EditorCamera->SetProjection(45, width / height, 0.1f, 1000.0f);  // TODO: camera settings
		}

		// TODO: Should be able to select any render result of any passes
		uint32_t renderResult = m_Renderer->GetRenderResultTextureID();
		//renderResult = m_Renderer->GetPipeline(RenderPipelineType::RPL_PostProcess)->GetRenderResult()->GetTextureID();
		ImGui::Image((ImTextureID)renderResult, ImGui::GetWindowSize(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		auto [mouseX, mouseY] = ImGui::GetMousePos();
		auto [windowPosX, windowPosY] = ImGui::GetWindowPos();
		int MouseX = mouseX - windowPosX, MouseY = mouseY - windowPosY; // Lower left is[0, 0]
		MouseY = height - MouseY;
		std::swap(m_MouseX, MouseX);
		std::swap(m_MouseY, MouseY);
		if (m_ShouldPickObject) {
			//m_ShouldPickObject = false;
			if (IsCursorInViewport()) {
				//m_Ray = GetRayFromScreenSpace(glm::vec2(float(MouseX), float(MouseY)),
				//	glm::vec2(float(m_ViewportWidth), float(m_ViewportHeight)),
				//	m_EditorCamera->GetPosition(),
				//	m_EditorCamera->GetProjectionInv(),
				//	m_EditorCamera->GetViewInv());
				//m_Renderer->GetPipeline(RenderPipelineType::RPL_DeferredShading)->GetRenderPassTarget(RenderPassType::SSAOGeo)->Bind();
				//s_PickPixelData = m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::SSAOGeo)->ReadPixel(TexType::Entity, MouseX, MouseY);
				//m_Renderer->GetCurrentRenderPipeline()->GetRenderPassTarget(RenderPassType::SSAOGeo)->Unbind();
			}
		}

		DrawGizmo();
		TryGetDragDropTarget();
		DrawLightIcons();
		ImGui::End();
	}

	void Viewport::DrawToolBarOverlay() {
		DrawToolBarAddObjectBtn();
		DrawToolBarRenderModeSelectionBtn();
		DrawManipulationToolBar();
	}

	void Viewport::DrawToolBarAddObjectBtn() {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.1f); // alpha value
		//ImGui::SetNextWindowSize(ImVec2{0.0f, 0.0f});
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
		static bool showFileExplorer = false;
		if (ImGui::BeginPopup("popup_menu")) {
			if (ImGui::MenuItem("Point Light")) {
				std::shared_ptr<AddLightSourceEvent> e = std::make_shared<AddLightSourceEvent>(LightType::PointLight);
				m_EventManager->PushBack(e);
			}
			if (ImGui::MenuItem("Distant Light")) {
				// TODO
			}
			if (ImGui::MenuItem("Environment Light")) {
				showFileExplorer = true;
				const auto& entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
				if (!m_EnvEntity.Valid()) {
					m_EnvEntity = Entity(entityManager->CreateEntity("Environment"));
				}
				if (!entityManager->HasComponent<EnvComponent>(m_EnvEntity)) {
					entityManager->AddComponent<EnvComponent>(m_EnvEntity);
				}
			}
			else {
				showFileExplorer = false;
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

		if (showFileExplorer) {
			fs::path file = FileExplorer::SelectFile(fs::current_path());
			if (!file.empty()) {
				showFileExplorer = false;
				if (file != ".") {
					AHO_CORE_INFO("{}", file.string());
					const auto& entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();

					Texture* hdr = new OpenGLTexture2D(file.string());
					hdr->SetTexType(TexType::HDR);
					static_cast<IBLPipeline*>(m_Renderer->GetPipeline(RenderPipelineType::RPL_IBL))->AddSphericalMap(hdr);
					//m_RP_IBL->AddSphericalMap(hdr);

					static_cast<DeferredShadingPipeline*>(m_Renderer->GetPipeline(RenderPipelineType::RPL_DeferredShading))->SetEnvLightState(true);
				}
			}
		}
	}

	static int s_CurrentMode = RenderMode::DefaultLit;
	void Viewport::DrawToolBarRenderModeSelectionBtn() {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.1f); // alpha value
		ImGui::SetNextWindowSize(ImVec2{ 0.0f, 0.0f });
		ImGui::Begin("SelectRenderMode##Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar();

		ImGuiStyle& style = ImGui::GetStyle();

		if (ImGui::ImageButton("SelectRenderMode##Button", (ImTextureID)m_AddIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize, g_ToolBarIconSize })) {
			ImGui::OpenPopup("RenderModePopup");
		}
		ImVec2 buttonMin = ImGui::GetItemRectMin(); // upper left
		ImVec2 buttonMax = ImGui::GetItemRectMax(); // lower right
		ImVec2 nxtPos = ImVec2(buttonMin.x, buttonMax.y);
		ImGui::SetNextWindowPos(nxtPos, ImGuiCond_Always);

		if (ImGui::BeginPopup("RenderModePopup")) {
			for (int i = 0; i < (int)RenderMode::ModeCount; ++i) {
				const char* modeName = nullptr;
				switch (i) {
				case Unlit:
					modeName = "Unlit";
					break;
				case DefaultLit:
					modeName = "Default Lit";
					break;
				case PathTracing:
					modeName = "Path Tracing";
					break;
				}

				if (ImGui::Selectable(modeName, s_CurrentMode == i, ImGuiSelectableFlags_DontClosePopups)) {
					s_CurrentMode = i;
				}

				if (s_CurrentMode == i) {
					ImGui::SetItemDefaultFocus();
				}
			}

			m_Renderer->SetRenderMode(RenderMode(s_CurrentMode));
			ImGui::EndPopup();
		}
		ImGui::End();
	}

	static bool s_IsClickingEventBlocked = false;
	static uint32_t s_PickPixelData = UINT_MAX;
	void Viewport::DrawGizmo() {
		bool m_Selected;
		Entity m_SelectedObject;
		m_Selected = false;
		if (m_LevelLayer->GetCurrentLevel()) {
			auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
			if (entityManager->IsEntityIDValid(s_PickPixelData)) {
				RendererGlobalState::g_SelectedEntityID = s_PickPixelData;
				RendererGlobalState::g_IsEntityIDValid = true;

				m_SelectedObject = Entity(static_cast<entt::entity>(s_PickPixelData));
				if (entityManager->HasComponent<TransformComponent>(m_SelectedObject)) {
					m_Selected = true;
					auto& tc = entityManager->GetComponent<TransformComponent>(m_SelectedObject);
					auto& translation = tc.GetTranslation();
					auto& scale = tc.GetScale();
					auto& rotation = tc.GetRotation();
					auto transform = tc.GetTransform();
					// Gizmo
					const auto& viewMat = m_EditorCamera->GetView();
					const auto& projMat = m_EditorCamera->GetProjection();
					ImGuizmo::SetOrthographic(false);
					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_ViewportWidth, m_ViewportHeight - ImGui::GetFrameHeight());

					// test these: ImGui::IsWindowHovered() && ImGui::IsWindowFocused()
					if (g_Operation != ImGuizmo::OPERATION::NONE) {
						s_IsClickingEventBlocked |= ImGuizmo::IsOver();
						ImGuizmo::Manipulate(glm::value_ptr(viewMat), glm::value_ptr(projMat),
							g_Operation,
							ImGuizmo::MODE::LOCAL,
							glm::value_ptr(transform));
						ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
					}
				}
			}
			else {
				RendererGlobalState::g_IsEntityIDValid = false;
			}
		}
	}

	void Viewport::DrawManipulationToolBar() {
		// Draws a translucent tool bar
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.5f, 0.5f, 0.5f, 0.2f));
		ImGui::Begin("ManipulationToolBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse/* | ImGuiWindowFlags_NoMove*/);
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		if (ImGui::ImageButton("selectionMode", (ImTextureID)m_CursorIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize },
			ImVec2(0, 0), ImVec2(1, 1),
			g_SelectedButton == ButtonType::Selection ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
			g_SelectedButton = ButtonType::Selection;
			g_Operation = ImGuizmo::OPERATION::NONE;
			m_ShouldPickObject = false;
			s_IsClickingEventBlocked = true;
		}
		ImGui::SameLine();
		auto& io = ImGui::GetIO();
		ImGui::PushFont(io.Fonts->Fonts[2]);
		if (ImGui::Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, ImVec2{ g_ToolBarIconSize, g_ToolBarIconSize })) {
			g_SelectedButton = ButtonType::Translation;
			g_Operation = ImGuizmo::OPERATION::TRANSLATE;
			m_ShouldPickObject = false;
			s_IsClickingEventBlocked = true;
		}
		ImGui::PopFont();
		//if (ImGui::ImageButton("translationMode", (ImTextureID)m_TranslationIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize },
		//	ImVec2(0, 0), ImVec2(1, 1),
		//	g_SelectedButton == ButtonType::Translation ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
		//	g_SelectedButton = ButtonType::Translation;
		//	g_Operation = ImGuizmo::OPERATION::TRANSLATE;
		//	m_ShouldPickObject = false;
		//	s_IsClickingEventBlocked = true;
		//}
		ImGui::SameLine();
		if (ImGui::ImageButton("rotationMode", (ImTextureID)m_RotationIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize }, ImVec2(0, 0), ImVec2(1, 1),
			g_SelectedButton == ButtonType::Rotation ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
			g_SelectedButton = ButtonType::Rotation;
			g_Operation = ImGuizmo::OPERATION::ROTATE;
			m_ShouldPickObject = false;
			s_IsClickingEventBlocked = true;
		}
		ImGui::SameLine();
		if (ImGui::ImageButton("scaleMode", (ImTextureID)m_ScaleIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize ,g_ToolBarIconSize }, ImVec2(0, 0), ImVec2(1, 1),
			g_SelectedButton == ButtonType::Scale ? ImVec4{ 0.529f, 0.808f, 0.922f, 0.8f } : ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f })) {
			g_SelectedButton = ButtonType::Scale;
			g_Operation = ImGuizmo::OPERATION::SCALE;
			m_ShouldPickObject = false;
			s_IsClickingEventBlocked = true;
		}
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(4);
		ImGui::End();
	}

	static const float s_LightVisSize = 60.0f;
	void Viewport::DrawLightIcons() {
		auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
		const auto& projMat = m_EditorCamera->GetProjection();
		const auto& viewMat = m_EditorCamera->GetView();
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
	
	void Viewport::TryGetDragDropTarget() {
		bool showPopup = false;
		static std::string droppedString;
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_MESH");
			if (payload) {
				const char* droppedData = static_cast<const char*>(payload->Data);
				droppedString = std::string(droppedData, payload->DataSize);
				showPopup = true;
				AHO_TRACE("Payload accepted! {}", droppedString);
			}
			ImGui::EndDragDropTarget();
		}
		if (showPopup) {
			ImGui::OpenPopup("Load Settings");
		}

		if (ImGui::BeginPopup("Load Settings")) {
			static bool staticMesh{ true };
			std::string prompt = "Mesh Type: " + std::string(staticMesh ? "Static Mesh" : "Skeletal Mesh");
			ImGui::Text(prompt.c_str());
			ImGui::Separator();
			ImGui::Checkbox("Static Mesh", &staticMesh);
			ImGui::Separator();

			static glm::vec3 translation(0.0f);
			static glm::vec3 scale(1.0f);
			static glm::vec3 rotation(0.0f);
			ImGui::Text("Import Settings");
			ImGui::DragFloat3("Translation##Importing", &translation[0], 0.01f);
			ImGui::DragFloat3("Scale##Importing", &scale[0], 0.01f);
			ImGui::DragFloat3("Rotation##Importing", &rotation[0], 0.01f);
			ImGui::Separator();

			if (ImGui::Button("Load") && !droppedString.empty()) {
				glm::mat4 preTransform = ComposeTransform(translation, rotation, scale);
				AHO_CORE_INFO("Loading asset from file : {}", droppedString);
				std::shared_ptr<AssetImportedEvent> event = std::make_shared<AssetImportedEvent>(droppedString, !staticMesh, preTransform);
				AHO_CORE_WARN("Pushing an AssetImportedEvent!");
				m_EventManager->PushBack(event);
				ImGui::CloseCurrentPopup();
				droppedString.clear();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
				droppedString.clear();
			}
			ImGui::EndPopup();
		}
	}
}
