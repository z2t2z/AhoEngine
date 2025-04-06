#include "AhoEditorLayer.h"
#include "FileExplorer.h"
#include <iomanip>
#include <entt.hpp>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Runtime/Core/Gui/IconsFontAwesome6.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"

namespace Aho {
	namespace fs = std::filesystem;

	static ImGuizmo::OPERATION g_Operation = ImGuizmo::OPERATION::NONE;

	glm::vec3 g_testPosition;


	AhoEditorLayer::AhoEditorLayer(LevelLayer* levellayer, ResourceLayer* resourceLayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager)
		: Layer("EditorLayer"), m_ResourceLayer(resourceLayer), m_LevelLayer(levellayer), m_EventManager(eventManager), m_Renderer(renderer), m_CameraManager(cameraManager) {
	
	}

	void AhoEditorLayer::OnAttach() {
		AHO_INFO("EditorLayer on attach");
		
		m_ContentBrowser.Initialize();
		m_PropertiesPanel.Initialize(m_LevelLayer, m_Renderer);
		m_HierachicalPanel.Initialize(m_LevelLayer);
		m_Viewport.Initialize(m_Renderer, m_LevelLayer, m_ResourceLayer, m_EventManager, m_CameraManager->GetMainEditorCamera());

	}
	
	void AhoEditorLayer::OnDetach() {
	}

	void AhoEditorLayer::OnUpdate(float deltaTime) {
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
				m_Renderer->SetCameraDirty();
			}
		}
		else if (m_CursorLocked) {
			m_CursorLocked = false;
			Input::UnlockCursor();
		}

		m_DeltaTime = deltaTime;
		auto& watcher = FileWatcher::getInstance();
		watcher.PollFiles(deltaTime);
	}

	void AhoEditorLayer::OnImGuiRender() {
		// Dock space settings and some basic settings
		{
			static bool showDemo = false;
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
					ImGui::MenuItem("ShowImGuiDemoWindow", NULL, &showDemo);
					ImGui::MenuItem("DebugView", NULL, &RendererGlobalState::g_ShowDebug);
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
			if (showDemo) {
				ImGui::ShowDemoWindow();
			}
		}

		static Entity selectedEntity;
		Entity entity = m_HierachicalPanel.Draw();
		if (entity.Valid()) {
			selectedEntity = entity;
		}
		m_ContentBrowser.Draw();
		m_PropertiesPanel.Draw(selectedEntity);
		m_Viewport.Draw();

		// Temporary testing functions
		TempSunDirControl();
		TempBVHControl();
	}

	void AhoEditorLayer::OnEvent(Event& e) {
		// Handle all input events here
		if (e.GetEventType() == EventType::MouseButtonPressed) {
			e.SetHandled();
			auto ee = (MouseButtonPressedEvent*)&e;
			if (m_Viewport.IsCursorInViewport() && !m_IsClickingEventBlocked && ee->GetMouseButton() == AHO_MOUSE_BUTTON_1) {
				m_ShouldPickObject = true;
			}
			else {
				m_IsClickingEventBlocked = false;
			}
		}
	}

	void AhoEditorLayer::AddEnvMap(const Texture* texture) {

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

	// TODO: need to integrate in a skyatmosphere component
	void AhoEditorLayer::TempSunDirControl() {
		auto skyPipeline = static_cast<RenderSkyPipeline*>(m_Renderer->GetPipeline(RenderPipelineType::RPL_RenderSky));
		auto shadingPipeline = static_cast<DeferredShadingPipeline*>(m_Renderer->GetPipeline(RenderPipelineType::RPL_RenderSky));

		// super strange bug
		auto& [yaw, pitch] = skyPipeline->GetSunYawPitch();
		auto sundir = skyPipeline->GetSunDir();
		ImGui::Begin("Temp Sky Control");
		ImGui::DragFloat("Yaw", &yaw, 0.01f, -3.14f, 3.14f);
		ImGui::DragFloat("Pitch", &pitch, 0.01f, -3.14f / 2, 3.14f / 2);
		glm::vec3 sunDir = glm::vec3(glm::cos(pitch) * glm::cos(yaw), glm::sin(pitch), glm::cos(pitch) * glm::sin(yaw));
		std::string text = std::to_string(sunDir.x) + " " + std::to_string(sunDir.y) + " " + std::to_string(sunDir.z);
		ImGui::Text(text.c_str());
		sunDir = glm::normalize(sunDir);
		skyPipeline->SetSunDir(sunDir);
		shadingPipeline->SetSunDir(sunDir);

		ImGui::End();
	}

	void AhoEditorLayer::TempBVHControl() {
		ImGui::Begin("Temp bvh control");
		auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
		auto view = entityManager->GetView<BVHComponent, TransformComponent>();
		int find = -1;
		
		if (ImGui::Button("Update BVH")) {
			bool dirty = false;
			view.each(
				[&dirty](auto entity, BVHComponent& bc, TransformComponent& tc) {
					if (tc.dirty) {
						tc.dirty = false;
						dirty = true;
						for (BVHi* bvh : bc.bvhs) {
							bvh->ApplyTransform(tc.GetTransform());
						}
					}
				});


			if (dirty) {
				BVHi& alts = m_LevelLayer->GetCurrentLevel()->GetTLAS();
				alts.UpdateTLAS();
				PathTracingPipeline* ptpl = static_cast<PathTracingPipeline*>(m_Renderer->GetPipeline(RenderPipelineType::RPL_PathTracing));
				ptpl->UpdateSSBO(m_LevelLayer->GetCurrentLevel());
			}

		}

		//if (ImGui::Button("GetData")) {
		//	std::vector<BVHNodei> data(1);
		//	SSBOManager::GetSubData<BVHNodei>(0, data, 0);
		//}
		//view.each(
		//	[&](auto entity, BVHComponent& bc, TransformComponent& tc) {
		//		ImGui::Text("EntityID: %d", static_cast<uint32_t>(entity));
		//		std::string showName = "Update BVH:" + std::to_string(static_cast<uint32_t>(entity));
		//		if (ImGui::Button(showName.c_str())) {
		//			//ScopedTimer timer(std::to_string(static_cast<uint32_t>(entity)));
		//			bc.bvh.ApplyTransform(tc.GetTransform());
		//		}
		//		
		//		if (find == -1 && m_ShouldPickObject) {
		//			auto cam = m_CameraManager->GetMainEditorCamera();
		//			{
		//				ScopedTimer timer("Idx Intersecting test");
		//				if (bc.bvh.Intersect(m_Ray)) {
		//					find = static_cast<uint32_t>(entity);
		//				}
		//			}

		//			// testing ptr version bvh
		//			//if (!intersectionResult) {
		//			//	ScopedTimer timer("PTR Intersecting test");
		//			//	intersectionResult = BVH::GetIntersection(m_Ray, root.get());
		//			//}
		//		
		//		}

		//		ImGui::Separator();
		//	});
	
		if (m_ShouldPickObject && Input::IsKeyPressed(AHO_KEY_LEFT_CONTROL)) {
			BVHi& alts = m_LevelLayer->GetCurrentLevel()->GetTLAS();
			ScopedTimer timer("Intersecting test");
			if (alts.Intersect(m_Ray)) {
				find = 1;
			}
		}


		if (find != -1) {
			AHO_CORE_TRACE("Intersecting {}", find);
		}

		m_ShouldPickObject = false;

		ImGui::End();
	}
}