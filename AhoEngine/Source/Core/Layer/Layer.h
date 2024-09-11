#pragma once

#include "Core/Core.h"
#include "Core/Events/Event.h"

#include "Core/Renderer/Framebuffer.h"

/*
	Layer�������Ϸ�����е�һ���㡣ÿ������Կ�������Ⱦ�������¼�������߼���һ���֡�
	���磬����Ϸ�У����ܻ���һ��UI�㡢һ����Ϸ�߼��㡢һ��������ȡ�
	ͨ�����ַ�ʽ����ͬ�Ĺ���ģ����Է���ɲ�ͬ�Ĳ�Σ����ӱ��ڹ���͵��ԡ�
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

