#pragma once

#include "Core/Core.h"
#include "Core/Events/Event.h"


/*
	Layer类代表游戏引擎中的一个层。每个层可以看作是渲染、处理事件或更新逻辑的一部分。
	例如，在游戏中，可能会有一个UI层、一个游戏逻辑层、一个背景层等。
	通过这种方式，不同的功能模块可以分离成不同的层次，更加便于管理和调试。
*/


namespace Aho {

	class AHO_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetDebugName() {
			return m_DebugName;
		}

	private:
		std::string m_DebugName;
	};

} // namespace Aho

