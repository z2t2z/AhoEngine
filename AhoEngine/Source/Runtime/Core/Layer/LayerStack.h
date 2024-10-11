#pragma once

#include "Runtime/Core/Core.h"
#include "Layer.h"

/*
	����Ϸ�����ͼ��Ӧ�ó����У�overlay�����Ӳ㣩ͨ��ָ��������������֮����ʾ�Ľ����ͼ��Ԫ�ء�
	������Ҫ���ڻ��������û����棨UI����������Ϣ���˵���HUD��Head-Up Display���Ȳ��볡��ֱ�ӽ��������ݡ�
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

