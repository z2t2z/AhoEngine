#pragma once

#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Core/Events/MouseEvent.h"
#include "Runtime/Core/Events/KeyEvent.h"
#include "Runtime/Core/Events/ApplicationEvent.h"


namespace Aho {

	class ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	
	};

} 

