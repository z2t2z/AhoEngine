#include "IamAho.h"

#include "AhoEditorLayer.h"
#include <filesystem>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>


namespace Aho {
	AhoEditorLayer::AhoEditorLayer() {
		m_Color = glm::vec4(1.0f);
	}

	void AhoEditorLayer::OnAttach() {
		// Set the active scene
		m_CameraManager = std::make_shared<CameraManager>();
		m_CameraManager->GetMainEditorCamera()->MoveBackward(1.0f);
		m_ActiveScene = std::make_shared<Scene>();
		
		// Temporary init shader here
		std::filesystem::path currentPath = std::filesystem::current_path();
		std::string path = currentPath.string() + "\\ShaderSrc\\shader.glsl";
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
		m_Cube = m_ActiveScene->CreateAObject("Cube");
		m_Cube.AddComponent<TransformComponent>();
		m_Cube.AddComponent<MeshComponent>(m_CubeVA);

		// temporary
		//std::string filePath = "D:/tcd/Sem2/Real-time-rendering/source/resources/models/sponza/sponza.obj";
		//std::string filePath = "D:/Projs/Piccolo/bin/asset/objects/basic/cube.obj";
		m_Manager = new AssetManagerEditor();
		std::string t = "D:/tcd/Sem2/Real-time-rendering/sem2/resources/models/Beriev_A50/BerievA50.obj";
		m_Manager->CreateAssetFromFile(t);
		//m_Test = m_ActiveScene->CreateAObject("Cube");
		//m_Test.AddComponent<MeshesComponent>(filePath);
	}

	void AhoEditorLayer::OnDetach() {

	}

	void AhoEditorLayer::OnUpdate(float deltaTime) {
		m_CameraManager->Update(deltaTime);

		//AHO_TRACE("{}", deltaTime);
		m_DeltaTime = deltaTime;
		m_Framebuffer->Bind();

		// TODO : This seems should not be here
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		m_ActiveScene->OnUpdateEditor(m_CameraManager->GetMainEditorCamera(), m_Shader, deltaTime);

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
		ImGui::End();

		// Viewport Window
		ImGui::Begin("Viewport");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		auto [width, height] = ImGui::GetContentRegionAvail();
		auto spec = m_Framebuffer->GetSpecification();
		if (spec.Width != width || spec.Height != height) {
			m_Framebuffer->Resize(width, height);
			m_CameraManager->GetMainEditorCamera()->SetProjection(45, width / height, 0.1f, 1000.0f);
		}

		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)textureID, ImVec2{ width, height });
		ImGui::End();
	}


	void AhoEditorLayer::OnEvent(Event& e) {
		EventDispatcher eventDispatcher(e);
	}

}