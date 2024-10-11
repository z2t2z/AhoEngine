#pragma once

#include "Runtime/Core/Core.h"
#include "Layer.h"

/*
	在游戏引擎和图形应用程序中，overlay（叠加层）通常指的是在其他内容之上显示的界面或图形元素。
	它们主要用于绘制诸如用户界面（UI）、调试信息、菜单、HUD（Head-Up Display）等不与场景直接交互的内容。
*/

namespace Aho {

	class AHO_API LayerStack {
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer (Layer* layer);
		void PopOverlay(Layer* layer);

		std::vector<Layer*>::iterator begin() {
			return m_Layers.begin();
		}
		std::vector<Layer*>::iterator end() {
			return m_Layers.end();
		}
		std::reverse_iterator<std::vector<Layer*>::iterator> rbegin() {
			return m_Layers.rbegin();
		}
		std::reverse_iterator<std::vector<Layer*>::iterator> rend() {
			return m_Layers.rend();
		}

	private:
		std::vector<Layer*> m_Layers;
		unsigned int m_LayerInsertIndex{ 0u };
	};

} // namespace Aho

