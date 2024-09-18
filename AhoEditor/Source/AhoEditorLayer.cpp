#include "AhoEditorLayer.h"

#include <filesystem>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>


namespace Aho {
	AhoEditorLayer::AhoEditorLayer() {
		m_Color = glm::vec4(1.0f);
	}

	// What if put this in the constructor?
	void AhoEditorLayer::OnAttach() {
		// Set the active scene
		m_ActiveScene = std::make_shared<Scene>();
		
		// Temporary init shader here
		std::filesystem::path currentPath = std::filesystem::current_path();
		std::string path = currentPath.string() + "\\ShaderSrc\\shader.glsl";
		AHO_TRACE(path);
		m_Shader = Shader::Create(path);

		Renderer::Init(m_Shader);
		m_Color = glm::vec4(0);
		// Temporary setting up viewport FBO
		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		auto rendererID = m_Shader->GerRendererID();
		fbSpec.rendererID = rendererID;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		// Temporary setting up Cube VAO
		m_CubeVA.reset(VertexArray::Create());
		float vertices[] = {
			// Front face
			-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,

			// Back face
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

			// Left face
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f,

			// Right face
			 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
			 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

			 // Bottom face
			 -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
			  0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
			  0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f,
			 -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f,

			 // Top face
			 -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
			 -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f
		};
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		BufferLayout layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" }
		};
		vertexBuffer->SetLayout(layout);
		m_CubeVA->AddVertexBuffer(vertexBuffer);
		
		unsigned int indices[] = {
			// Front face
			0, 1, 2,
			2, 3, 0,
			// Back face
			4, 5, 6,
			6, 7, 4,
			// Left face
			8, 9, 10,
			10, 11, 8,
			// Right face
			12, 13, 14,
			14, 15, 12,
			// Bottom face
			16, 17, 18,
			18, 19, 16,
			// Top face
			20, 21, 22,
			22, 23, 20
		};
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_CubeVA->SetIndexBuffer(indexBuffer);

		// Entities
		m_Cube = m_ActiveScene->CreateEntity("Cube");
		m_Cube.AddComponent<TransformComponent>();
		m_Cube.AddComponent<MeshComponent>(m_CubeVA);
		
		m_Camera = new Camera();
		m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
		m_CameraEntity.AddComponent<TransformComponent>();
		m_CameraEntity.AddComponent<CameraComponent>(m_Camera, true);

		// temporary
		std::string filePath = "D:/tcd/Sem2/Real-time-rendering/source/resources/models/sponza/sponza.obj";
		m_Test = m_ActiveScene->CreateEntity("Sponza");
		m_Test.AddComponent<MeshesComponent>(filePath);
	}

	void AhoEditorLayer::OnDetach() {
	}

	void AhoEditorLayer::OnUpdate() {
		m_Framebuffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		m_ActiveScene->OnUpdateEditor(m_Camera, m_Shader, m_Color);

		m_Framebuffer->Unbind();
	}

	void AhoEditorLayer::OnImGuiRender() {
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

		// Editor panel
		ImGui::Begin("Editor Panel");
		ImGui::Text("This is the editor panel");
		auto& tc = m_CameraEntity.GetComponent<TransformComponent>();
		ImGui::SliderFloat3("Camera Trnasform", glm::value_ptr(tc.Translation), -10.0f, 10.0f);
		ImGui::SliderFloat4("Debug Color", glm::value_ptr(m_Color), 0.0f, 1.0f);
		ImGui::End();

		// Viewport Window, resizeing, seems incorrect?
		ImGui::Begin("Viewport");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		auto [width, height] = ImGui::GetContentRegionAvail();
		auto spec = m_Framebuffer->GetSpecification();
		if (spec.Width != width || spec.Height != height) {
			AHO_WARN("Resizeing viewport to: {0} {1}", width, height);
			m_Framebuffer->Resize(width, height);
		}

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ width, height });
		ImGui::End();
	}

	void AhoEditorLayer::OnEvent(Event& e) {
	}

	bool AhoEditorLayer::OnKeyPressed(KeyPressedEvent& e) {
		return false;
	}

}