#pragma once
#include "IamAho.h"

namespace Aho {
	class ViewportPanel {
	public:
		ViewportPanel(std::shared_ptr<Framebuffer> fbo, EventManager* em, std::shared_ptr<CameraManager> cm)
			: m_FBO(fbo), m_EventManager(em), m_CameraManager(cm) { }
		bool DrawPanel();
		void SetFramebuffer(std::shared_ptr<Framebuffer> fbo) { m_FBO = fbo; }
		void SetObjectPicking() { m_PickObject = true; }
	private:
		bool m_PickObject{ false };
		bool m_CursorInViewport{ false };
		uint32_t m_PickData{ 0u };
		float m_Offset{ 50.0f };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager{ nullptr };
		std::shared_ptr<Framebuffer> m_FBO;
	};
} // namespace Aho