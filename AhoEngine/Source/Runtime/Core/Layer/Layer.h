#pragma once

#include "Runtime/Core/Core.h"
#include "Runtime/Core/Events/Event.h"

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
		inline const std::string& GetDebugName() {
			return m_DebugName;
		}

	protected:
		std::string m_DebugName;
	};

} // namespace Aho

