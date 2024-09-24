#pragma once

#include "Core/Core.h"
#include "Core/Events/Event.h"

#include "Core/Renderer/Framebuffer.h"

namespace Aho {

	class AHO_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float deltaTime) {}

		// TODO:
		// virtual void OnLogicUpdate() {}
		// virtual void OnLogicUpdate() {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImGuiRender() {}

		// TO BE DELETED!
		//virtual void OnImGuiRender(std::shared_ptr<Framebuffer> m_Framebuffer) {}

		inline const std::string& GetDebugName() {
			return m_DebugName;
		}

	protected:
		std::string m_DebugName;
	};

} // namespace Aho

