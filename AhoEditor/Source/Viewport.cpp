#include "Viewport.h"
#include "FileExplorer.h"
#include "EditorUI/ImGuiHelpers.h"
#include "EditorUI/EditorGlobalContext.h"
#include "Runtime/Core/Events/EngineEvents.h"
#include "Runtime/Core/Geometry/VirtualMesh/VirtualMesh.h"
#include "Runtime/Core/Gui/IconsFontAwesome6.h"
#include "Runtime/Resource/Asset/AssetLoadOptions.h"
#include "Runtime/Resource/ResourceManager.h"
#include "Runtime/Resource/Asset/TextureAsset.h"
#include "Runtime/Function/Renderer/Texture/_Texture.h"
#include "Runtime/Function/Renderer/IBL/IBLManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Aho {
	static ImGuizmo::OPERATION g_Operation = ImGuizmo::OPERATION::NONE;

	Viewport::Viewport() {
	}

	void Viewport::Initialize(Renderer* renderer, LevelLayer* levelLayer, EventManager* manager, const std::shared_ptr<Camera>& camera) {
		m_Renderer = renderer;
		m_LevelLayer = levelLayer;
		m_EventManager = manager;
		m_EditorCamera = camera;

		namespace fs = std::filesystem;
		auto path = fs::current_path();
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		{
			std::shared_ptr<TextureAsset> textureAsset = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions((path / "Asset" / "EngineAsset" / "Icons" / "light-bulb.png").string()));
			std::shared_ptr<_Texture> tex = g_RuntimeGlobalCtx.m_Resourcemanager->LoadGPUTexture(textureAsset);
			m_LightIcon = tex.get();
		}
		{
			std::shared_ptr<TextureAsset> textureAsset = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions((path / "Asset" / "EngineAsset" / "Icons" / "plusicon.png").string()));
			std::shared_ptr<_Texture> tex = g_RuntimeGlobalCtx.m_Resourcemanager->LoadGPUTexture(textureAsset);
			m_AddIcon = tex.get();
		}
	}

	void Viewport::Draw() {
		DrawMainViewport();
		DrawToolBarOverlay();
		DrawLightIcons(); // visual debug
	}

	// Handle object picking here
	void Viewport::DrawMainViewport() {
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::Begin("Viewport", nullptr, window_flags);
		ImGui::PopStyleVar();

		auto [width, height] = ImGui::GetWindowSize();
		m_ViewportWidth = width, m_ViewportHeight = height;
		if (m_Renderer->OnViewportResize(width, height)) {
			//m_EditorCamera->SetAspectRatio(width / height);
			m_EditorCamera->SetProjection(45, width / height, 0.1f, 1000.0f);  // TODO: camera settings
		}

		ImVec2 localPos = ImGui::GetMousePos() - ImGui::GetWindowPos();
		int x = localPos.x, y = localPos.y;
		m_MouseX = x, m_MouseY = y;
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
			&& !ImGui::IsAnyItemHovered()
			&& !ImGuizmo::IsOver()
			&& x >= 0 && y >= 0 && x < m_ViewportWidth && y < m_ViewportHeight) {
			g_EditorGlobalCtx.RequestPicking(x, m_ViewportHeight - y);
		}

		uint32_t renderResult = m_Renderer->GetViewportDisplayTextureID();

		// Some debugs here
		auto iblManager = g_RuntimeGlobalCtx.m_IBLManager;
		auto activeIBLEntity = g_RuntimeGlobalCtx.m_IBLManager->GetActiveIBL();
		if (g_RuntimeGlobalCtx.m_EntityManager->HasComponent<IBLComponent>(activeIBLEntity)) {
			auto& iblComp = g_RuntimeGlobalCtx.m_EntityManager->GetComponent<IBLComponent>(activeIBLEntity);
			renderResult = iblComp.Prefilter->GetTextureID();
		}

		ImGui::Image((ImTextureID)(intptr_t)renderResult, ImGui::GetWindowSize(), ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		DrawGizmo();
		TryGetDragDropTarget();
		ImGui::End();
	}

	void Viewport::DrawToolBarOverlay() {
		DrawToolBarAddObjectBtn();
		DrawToolBarRenderModeSelectionBtn();
		DrawManipulationToolBar();
	}

	void Viewport::DrawToolBarAddObjectBtn() {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.1f); // alpha value
		ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
		ImGui::PopStyleVar();

		ImGuiStyle& style = ImGui::GetStyle();

		static const float g_ToolBarIconSize = 22.0f;
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
			}
			if (ImGui::MenuItem("Area Light")) {
				// TODO
				//namespace fs = std::filesystem;
				//auto path = fs::current_path() / "Asset" / "Basic";
				//auto recMesh = AssetCreator::MeshAssetCreater((path / "2x2Square.obj").string());
				//m_LevelLayer->AddStaticMeshToScene(recMesh, "Area Light", std::make_shared<AreaLight>());
			}
			if (ImGui::MenuItem("Distant Light")) {
				// TODO
			}
			if (ImGui::MenuItem("Spot Light")) {
				// TODO
			}
			if (ImGui::MenuItem("Sky Atmospheric")) {
				auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
				if (ecs->GetView<AtmosphereParametersComponent>().size() > 0) {
					return;
				}
				auto skyEntity = ecs->CreateEntity();
				glm::vec3 sunRotation(60.0f, -90.0f, 0.0f);
				float theta = glm::radians(sunRotation.x), phi = glm::radians(sunRotation.y);
				glm::vec3 sunDir = normalize(glm::vec3(glm::sin(theta) * glm::cos(phi), glm::cos(theta), glm::sin(theta) * glm::sin(phi)));
				ecs->AddComponent<AtmosphereParametersComponent>(skyEntity);
				ecs->AddComponent<GameObjectComponent>(skyEntity, "GO_SkyAtmosphere");

				LightComponent& lcomp = ecs->AddComponent<LightComponent>(skyEntity);
				lcomp.index = g_RuntimeGlobalCtx.m_IndexAllocator->AcquireIndex<DirectionalLight>();
				std::shared_ptr<DirectionalLight> light = std::make_shared<DirectionalLight>();
				light->SetDirection(sunDir);
				lcomp.light = std::move(light);
				ecs->AddComponent<LightDirtyTagComponent>(skyEntity);

				// Modify shader features(add #defines accordingly) and recompile shaders
				EngineEvents::OnShaderFeatureChanged.Dispatch(ShaderUsage::DeferredShading, ShaderFeature::FEATURE_SKY_ATMOSPHERIC, ShaderFeature::FEATURE_IBL);
			}

			if (ImGui::MenuItem("Environment Light")) {
				showFileExplorer = true;
			} else {
				showFileExplorer = false;
			}
			if (ImGui::MenuItem("Cube")) {
			}
			if (ImGui::MenuItem("Sphere")) {
			}
			if (ImGui::MenuItem("Plane")) {
			}
			if (ImGui::MenuItem("Cylinder")) {
			}
			ImGui::EndPopup();
		}
		ImGui::End();

		if (showFileExplorer) {
			fs::path file = FileExplorer::SelectFile(fs::current_path());
			if (!file.empty()) {
				showFileExplorer = false;
				auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
				auto resManager = g_RuntimeGlobalCtx.m_Resourcemanager;
				auto assetManager = g_RuntimeGlobalCtx.m_AssetManager;
				auto textureAsset = assetManager->_LoadAsset<TextureAsset>(ecs, TextureOptions(file.string()));
				Entity IBLEntity = resManager->LoadIBL(textureAsset);
				g_RuntimeGlobalCtx.m_IBLManager->SetActiveIBL(IBLEntity);

				EngineEvents::OnShaderFeatureChanged.Dispatch(ShaderUsage::DeferredShading, ShaderFeature::FEATURE_IBL, ShaderFeature::FEATURE_SKY_ATMOSPHERIC);
				EngineEvents::OnShaderFeatureChanged.Dispatch(ShaderUsage::PathTracing, ShaderFeature::FEATURE_IBL, ShaderFeature::FEATURE_SKY_ATMOSPHERIC);
			}
		}
	}

	void Viewport::DrawToolBarRenderModeSelectionBtn() {
		static int s_CurrentMode = RenderMode::DefaultLit;
		static int s_CurrentBufferIndex = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.1f); // alpha value
		ImGui::SetNextWindowSize(ImVec2{ 0.0f, 0.0f });
		ImGui::Begin("SelectRenderMode##Window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PopStyleVar();

		ImGuiStyle& style = ImGui::GetStyle();

		static const float g_ToolBarIconSize = 22.0f;
		if (ImGui::ImageButton("SelectRenderMode##Button", (ImTextureID)m_AddIcon->GetTextureID(), ImVec2{ g_ToolBarIconSize, g_ToolBarIconSize })) {
			ImGui::OpenPopup("RenderModePopup");
		}
		ImVec2 buttonMin = ImGui::GetItemRectMin(); // upper left
		ImVec2 buttonMax = ImGui::GetItemRectMax(); // lower right
		ImVec2 nxtPos = ImVec2(buttonMin.x, buttonMax.y);
		ImGui::SetNextWindowPos(nxtPos, ImGuiCond_Always);

		if (ImGui::BeginPopup("RenderModePopup")) {
			// === DefaultLit ===
			if (ImGui::Selectable("DefaultLit", s_CurrentMode == RenderMode::DefaultLit)) {
				s_CurrentMode = RenderMode::DefaultLit;
				m_Renderer->SetRenderMode(RenderMode::DefaultLit);
			}

			// === PathTracing ===
			if (ImGui::Selectable("PathTracing", s_CurrentMode == RenderMode::PathTracing)) {
				s_CurrentMode = RenderMode::PathTracing;
				m_Renderer->SetRenderMode(RenderMode::PathTracing);
			}

			// === BufferVisual (open sub-popup on hover) ===
			ImGui::Separator();
			bool hoveredBufferVisual = false;
			if (ImGui::Selectable("BufferVisual", s_CurrentMode == RenderMode::BufferVisual)) {
				// Don't select it yet ¡ª wait until buffer is chosen
			}

			if (ImGui::IsItemHovered()) {
				hoveredBufferVisual = true;
				ImGui::OpenPopup("BufferVisualPopup");
			}

			ImGui::SetNextWindowPos(ImGui::GetItemRectMax());
			// ==== Sub-Popup: BufferVisual Options ====
			if (ImGui::BeginPopup("BufferVisualPopup")) {
				ImGui::Text("Select Buffer:");
				ImGui::Separator();
				auto buffers = g_RuntimeGlobalCtx.m_Resourcemanager->GetAllBufferTextures();
				for (int i = 0; i < buffers.size(); ++i) {
					if (ImGui::Selectable(buffers[i]->GetLabel().c_str(), s_CurrentBufferIndex == i)) {
						s_CurrentMode = RenderMode::BufferVisual;
						s_CurrentBufferIndex = i;
						ImGui::CloseCurrentPopup();             // Close buffer list
						ImGui::CloseCurrentPopup();             // Close parent menu
						g_RuntimeGlobalCtx.m_Renderer->SetViewportDisplayTextureBuffer(buffers[i]);
					}
				}
				ImGui::EndPopup();
			}

			ImGui::EndPopup();
		}
		ImGui::End();
	}

	void Viewport::DrawGizmo() {
		if (!g_EditorGlobalCtx.HasActiveSelected())
			return;

		const Entity& selected = g_EditorGlobalCtx.GetSelectedEntity();
		auto ecs = g_RuntimeGlobalCtx.m_EntityManager;
		if (!ecs->HasComponent<_TransformComponent>(selected))
			return;

		_TransformComponent& transformComp = ecs->GetComponent<_TransformComponent>(selected);
		auto& translation = transformComp.Translation;
		auto& scale = transformComp.Scale;
		auto& rotation = transformComp.Rotation;
		auto transform = transformComp.GetTransform();

		// Gizmo
		const auto& viewMat = m_EditorCamera->GetView();
		const auto& projMat = m_EditorCamera->GetProjection();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_ViewportWidth, m_ViewportHeight - ImGui::GetFrameHeight());

		if (g_Operation != ImGuizmo::OPERATION::NONE) {
			bool changed = ImGuizmo::Manipulate(glm::value_ptr(viewMat), glm::value_ptr(projMat),
				g_Operation,
				ImGuizmo::MODE::LOCAL,
				glm::value_ptr(transform));
			if (changed) {
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
				transformComp.Dirty = true;
			}
		}
	}

	void Viewport::DrawManipulationToolBar() {
		static float windowPadding[2] = { 13.15f, -0.01f };
		static float windowBorderSize = 0.1f;
		static float windowRounding = 20.0f;
		static float windowMinSize[2] = { 0.0, 0.0 };
		static float btnSize[2] = { 25.0f, 27.5f };
		static float btn0Align[2] = { 2.450f, 0.750f };
		static float frameRounding = 0.0f;
		static float sameLineParam[2] = { 0.0f, 0.0f };
		auto _Debug = [&]() -> void {
			ImGui::Begin("_Debug");
			ImGui::DragFloat2("_padding", &windowPadding[0], 0.01, 0.0);
			ImGui::DragFloat("_border size", &windowBorderSize, 0.01, 0.0);
			ImGui::DragFloat("_rounding", &windowRounding, 0.01, 0.0);
			ImGui::DragFloat2("_minSize", &windowMinSize[0], 0.01, 0.0);
			ImGui::DragFloat2("_btnSize", &btnSize[0], 0.01, 0.0);
			ImGui::DragFloat("_frameRounding", &frameRounding, 0.01, 0.0);
			ImGui::DragFloat2("_btn0Align", &btn0Align[0], 0.01, 0.0);
			ImGui::DragFloat2("_sameLineParam", &sameLineParam[0], 0.01, -1.0, 1.0);
			ImGui::End();
			};
		//_Debug();

		static ImVec4 activeBtnColor = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

		struct UI_Button {
			UI_Button(const std::string& name, const ImGuizmo::OPERATION& oper)
				: name(name), oper(oper) {
			}
			std::string name;
			ImGuizmo::OPERATION oper;
		};

		const static std::vector<UI_Button> uiBtns = {
			UI_Button(ICON_FA_ARROW_POINTER, ImGuizmo::OPERATION::NONE),
			UI_Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, ImGuizmo::OPERATION::TRANSLATE),
			UI_Button(ICON_FA_ROTATE, ImGuizmo::OPERATION::ROTATE),
			UI_Button(ICON_FA_UP_RIGHT_AND_DOWN_LEFT_FROM_CENTER, ImGuizmo::OPERATION::SCALE),
		};
		static size_t activeBtnIdx = 0;

		// Draws a translucent tool bar
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(windowPadding[0], windowPadding[1]));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, windowBorderSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, windowRounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(windowMinSize[0], windowMinSize[1]));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.5f, 0.5f, 0.5f, 0.2f));

		ImGui::Begin("ManipulationToolBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);

		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::PushStyleColor(ImGuiCol_Button, activeBtnColor);
		//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, activeBtnColor);

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(btn0Align[0], btn0Align[1]));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, frameRounding);

		for (size_t i = 0; i < uiBtns.size(); i++) {
			auto& btn = uiBtns[i];
			activeBtnColor.w = (activeBtnIdx == i) ? 1.0f : 0.0f;
			ImGui::PushStyleColor(ImGuiCol_Button, activeBtnColor);
			ImGui::SameLine(sameLineParam[0], sameLineParam[1]);
			if (ImGui::Button(btn.name.c_str(), ImVec2{ btnSize[0], btnSize[1] })) {
				activeBtnIdx = i;
				g_Operation = btn.oper;
			}
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

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
			static bool buildBvh{ true };
			static bool virtualMesh{ false };
			std::string prompt = "Mesh Type: " + std::string(staticMesh ? "Static Mesh" : "Skeletal Mesh");
			ImGui::Text(prompt.c_str());
			ImGui::Separator();
			ImGui::Checkbox("Static Mesh", &staticMesh);
			ImGui::Separator();
			ImGui::Checkbox("Build BVH", &buildBvh);
			ImGui::Separator();
			ImGui::Checkbox("Build Virtual Mesh", &virtualMesh);
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
				std::string loadPath = droppedString;
				glm::mat4 preTransform = ComposeTransform(translation, rotation, scale);
				MeshOptions loadOptions(loadPath); loadOptions.PreTransform = preTransform; loadOptions.BuildBVH = buildBvh; loadOptions.IsSkeletalMesh = !staticMesh;

				auto& ecs = g_RuntimeGlobalCtx.m_EntityManager;
				auto resourceManager = g_RuntimeGlobalCtx.m_Resourcemanager;

				std::shared_ptr<MeshAsset> ms = g_RuntimeGlobalCtx.m_AssetManager->_LoadAsset<MeshAsset>(ecs, loadOptions);

				// ---- Virtual Mesh Loading Debug Test ----
				//const Mesh& mesh = ms->GetMesh();
				//std::vector<Meshlet> meshlets = VirtualMeshBuilder::BuildMeshlets(mesh.indexBuffer);
				////VirtualMeshBuilder::ExportMeshletsAsOBJ(meshlets, mesh.vertexBuffer, loadPath + "_meshlet.obj");

				//auto lod1 = VirtualMeshBuilder::BuildLODLevel(meshlets, 4);
				//VirtualMeshBuilder::ExportMeshletsAsOBJ(lod1, mesh.vertexBuffer, loadPath + "_lod1.obj");

				//auto lod2 = VirtualMeshBuilder::BuildLODLevel(lod1, 4);
				//VirtualMeshBuilder::ExportMeshletsAsOBJ(lod2, mesh.vertexBuffer, loadPath + "_lod2.obj");

				//auto lod3 = VirtualMeshBuilder::BuildLODLevel(lod2, 4);
				//VirtualMeshBuilder::ExportMeshletsAsOBJ(lod3, mesh.vertexBuffer, loadPath + "_lod3.obj");

				//auto lod4 = VirtualMeshBuilder::BuildLODLevel(lod3, 4);
				//VirtualMeshBuilder::ExportMeshletsAsOBJ(lod4, mesh.vertexBuffer, loadPath + "_lod4.obj");
				// ------------------------------------------

				std::string name = "GO_" + ms->GetName();
				Entity go = resourceManager->CreateGameObject(name);
				GameObjectComponent& goComp = ecs->GetComponent<GameObjectComponent>(go);

				// TODO: Put these in resource manager??
				auto view = ecs->GetView<MeshAssetComponent, MaterialRefComponent>(Exclude<VertexArrayComponent>); // Need testing
				view.each(
					[&](Entity entity, MeshAssetComponent& mc, MaterialRefComponent& matRefc) {
						goComp.children.push_back(entity);
						GameObjectComponent& childGoComp = ecs->AddComponent<GameObjectComponent>(entity, mc.asset->GetName());
						childGoComp.parent = go;
						std::shared_ptr<VertexArray> vao = resourceManager->LoadVAO(mc.asset);
						ecs->AddComponent<VertexArrayComponent>(entity, vao.get());
						auto& matAssetComponent = ecs->GetComponent<MaterialAssetComponent>(matRefc.MaterialRefEntity);
						ecs->AddComponent<_MaterialComponent>(entity, matAssetComponent.asset);
						ecs->AddComponent<_TransformComponent>(entity);
					}
				);

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