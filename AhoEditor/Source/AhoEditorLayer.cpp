#include "IamAho.h"
#include "AhoEditorLayer.h"
#include <filesystem>
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	AhoEditorLayer::AhoEditorLayer(Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager) 
		: m_Renderer(renderer), m_CameraManager(cameraManager) {

	}

	void AhoEditorLayer::OnAttach() {
		AHO_INFO("Editor on attach");
		// Set the active scene
		//m_ActiveScene = std::make_shared<Scene>();
		//m_Panel = std::make_unique<SceneHierarchyPanel>(m_ActiveScene);

		// Temporary init shader here
		std::filesystem::path currentPath = std::filesystem::current_path();
		m_FileWatcher.SetCallback(std::bind(&AhoEditorLayer::OnFileChanged, this, std::placeholders::_1));
		m_FileWatcher.AddFileToWatch(currentPath / "ShaderSrc" / "Shader.glsl");
		

#define LOAD_MODEL 0
#if LOAD_MODEL
		{
			std::filesystem::path newPath = currentPath / "Asset" / "sponzaFBX" / "sponza.fbx";
			std::shared_ptr<StaticMesh> res = std::make_shared<StaticMesh>();
			AssetManager::LoadAsset<StaticMesh>(newPath, *res);
			AObject plane = m_ActiveScene->CreateAObject("Plane");
			plane.AddComponent<EntityComponent>();
			for (const auto& meshInfo : *res) {
				std::shared_ptr<VertexArray> vao;
				vao.reset(VertexArray::Create());
				vao->Init(meshInfo);
				auto meshEntity = m_ActiveScene->CreateAObject();
				meshEntity.AddComponent<MeshComponent>(vao, static_cast<uint32_t>(meshEntity.GetEntityHandle()));
				meshEntity.AddComponent<TransformComponent>();
				if (meshInfo->materialInfo.HasMaterial()) {
					auto matEntity = m_ActiveScene->CreateAObject();
					std::shared_ptr<Material> mat = std::make_shared<Material>();
					for (const auto& albedo : meshInfo->materialInfo.Albedo) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(albedo);
						tex->SetTextureType(TextureType::Diffuse);
						mat->AddTexture(tex);
						if (tex->IsLoaded()) {
							// TODO: Cache the loaded texture
						}
					}
					for (const auto& normal : meshInfo->materialInfo.Normal) {
						std::shared_ptr<Texture2D> tex = Texture2D::Create(normal);
						tex->SetTextureType(TextureType::Normal);
						mat->AddTexture(tex);
						if (tex->IsLoaded()) {

						}
					}
					meshEntity.AddComponent<MaterialComponent>(mat);
				}
				plane.GetComponent<EntityComponent>().meshEntities.push_back(meshEntity.GetEntityHandle());
			}
		}
#endif
	}

	void AhoEditorLayer::OnDetach() {
	}

	void AhoEditorLayer::OnUpdate(float deltaTime) {
		m_CameraManager->Update(deltaTime);
		m_DeltaTime = deltaTime;
		m_FileWatcher.PollFiles();
	}

	glm::mat4 transf = glm::mat4(1.0f);

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
		auto FBO = m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->GetRenderTarget();
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
		// Gizmos
		{
			//auto [x, y] = Input::GetMousePosition();
			//auto [w, h] = m_Panel->GetPenalSize();
			//x -= w + 10.0f, y -= 45.0f; // TODO: Think of a better way
			//if (false and Input::IsMouseButtonPressed(AHO_MOUSE_BUTTON_2)) {
			//	m_PickingFBO->Bind();
			//	if (x >= 0 && y >= 0 && x < width && y < height) {
			//		uint32_t readData = m_PickingFBO->ReadPixel(0, x, y);
			//		if (readData == 0u) {
			//			m_Selected = entt::null;
			//		}
			//		else {
			//			m_Selected = static_cast<entt::entity>(readData);
			//		}
			//	}
			//	else {
			//		m_Selected = entt::null;
			//	}
			//	m_PickingFBO->Unbind();
			//}
			//if (m_Selected != entt::null) {
			//	ImGuizmo::SetOrthographic(false);
			//	ImGuizmo::SetDrawlist();
			//	ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, width, height);

			//	const auto& view = m_CameraManager->GetMainEditorCamera()->GetView();
			//	const auto& proj = m_CameraManager->GetMainEditorCamera()->GetProjection();
			//	auto transform = m_ActiveScene->m_Registry.get<TransformComponent>(m_Selected).GetTransform();
			//	ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(transf));

			//	// Is snapping
			//	bool snapping = Input::IsKeyPressed(AHO_KEY_LEFT_CONTROL);
			//}
		}
		ImGui::End();
	}


	void AhoEditorLayer::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<FileChangedEvent>(std::bind(&AhoEditorLayer::OnFileChanged, this, std::placeholders::_1));
	}

	bool AhoEditorLayer::OnFileChanged(FileChangedEvent& e) {
		if (e.GetEventType() == EventType::FileChanged) {
			const auto& FileName = e.GetFilePath();
			if (!FileName.empty()) {
				auto newShader = Shader::Create(FileName);
				if (newShader->IsCompiled()) {
					//m_Shader = std::move(newShader);
					m_Renderer->GetCurrentRenderPipeline()->GetRenderPass(0)->SetShader(std::move(newShader));
				}
			}
		}
		return true;
	}
}