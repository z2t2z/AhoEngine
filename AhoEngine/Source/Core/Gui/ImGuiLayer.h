#pragma once

#include "Core/Layer/Layer.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/KeyEvent.h"
#include "Core/Events/ApplicationEvent.h"

namespace Aho {

	class AHO_API ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& event) override;

	private:
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);

		bool OnMouseScrolledEvent(MouseScrolledEvent& event);
		
		bool OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event);

		bool OnMouseMovedEvent(MouseMovedEvent& event);

		bool OnKeyPressedEvent(KeyPressedEvent& event);
		
		bool OnKeyReleasedEvent(KeyReleasedEvent& event);
		
		bool OnKeyTypedEvent(KeyTypedEvent& event);

		bool OnWindowResizeEvent(WindowResizeEvent& event);

	private:
		float m_Time;
	
	};

} 

