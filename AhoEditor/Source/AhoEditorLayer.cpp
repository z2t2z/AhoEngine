#include "AhoEditorLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec3 a_Normal;

			out vec3 v_Position;
			out vec3 v_Normal;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Model; // TODO
			
			void main()
			{
				v_Position = (u_Model * vec4(a_Position, 1.0)).xyz;
				gl_Position = u_ViewProjection * vec4(v_Position, 1.0);
				v_Normal = a_Normal;
			}
		)";

	std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec3 v_Normal;

			uniform vec4 u_color;

			void main()
			{
				//color = vec4(v_Position * 0.5 + 0.5, 1.0);
				//color = vec4(1.0, 0.0, 0.0, 0.0);
				color = u_color;
			}
		)";
	AhoEditorLayer::AhoEditorLayer() {
	}

	// What if put this in the constructor?
	void AhoEditorLayer::OnAttach() {
		// Set the active scene
		m_ActiveScene = std::make_shared<Scene>();
		
		// Temporary init shader here
		m_Shader = Shader::Create("temp", vertexSrc, fragmentSrc);
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
		float vertices[8 * 6] = {
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
			4, 0, 3,   
			3, 7, 4,   
			// Right face
			1, 5, 6,   
			6, 2, 1,   
			// Top face
			3, 2, 6,   
			6, 7, 3,   
			// Bottom face
			0, 4, 5,   
			5, 1, 0    
		};
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_CubeVA->SetIndexBuffer(indexBuffer);

		// Entities
		//m_Test = m_ActiveScene->CreateEntity("Test");
		//m_Test.AddComponent<TagComponent>();
		m_Cube = m_ActiveScene->CreateEntity("Cube");
		m_Cube.AddComponent<TransformComponent>();
		m_Cube.AddComponent<MeshComponent>(m_CubeVA);
		
		m_Camera = new Camera();
		m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
		m_CameraEntity.AddComponent<TransformComponent>();
		m_CameraEntity.AddComponent<CameraComponent>(m_Camera, true);
	}

	void AhoEditorLayer::OnDetach() {
	}

	void AhoEditorLayer::OnUpdate() {
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		m_Framebuffer->Bind();

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
		auto& translation = tc.Translation;
		auto transform = tc.GetTransform();
		ImGui::SliderFloat3("Camera Trnasform", glm::value_ptr(translation), -10.0f, 10.0f);
		ImGui::SliderFloat4("Debug Color", glm::value_ptr(m_Color), 0.0f, 1.0f);
		tc.Translation = translation;
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