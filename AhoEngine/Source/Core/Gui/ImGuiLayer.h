#pragma once

#include "Core/Layer/Layer.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/ApplicationEvent.h"

#include "Core/Renderer/Framebuffer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Aho {

	class AHO_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		virtual void OnImGuiRender(std::shared_ptr<Framebuffer> m_Framebuffer) override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	
	};

} 

