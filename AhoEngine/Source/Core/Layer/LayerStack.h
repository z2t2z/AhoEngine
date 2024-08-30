#pragma once

#include "Core/Core.h"
#include "Layer.h"

/*
	����Ϸ�����ͼ��Ӧ�ó����У�overlay�����Ӳ㣩ͨ��ָ��������������֮����ʾ�Ľ����ͼ��Ԫ�ء�
	������Ҫ���ڻ��������û����棨UI����������Ϣ���˵���HUD��Head-Up Display���Ȳ��볡��ֱ�ӽ��������ݡ�
*/

namespace Aho {

	class LayerStack {
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		std::vector<Layer*>::iterator begin() {
			return m_Layers.begin();
		}

		std::vector<Layer*>::iterator end() {
			return m_Layers.end();
		}

	private:
		std::vector<Layer*> m_Layers;
		std::vector<Layer*>::iterator m_LayerInsert;
	};

} // namespace Aho

