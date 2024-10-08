#pragma once

#include "Runtime/Core/Layer/Layer.h"
#include "FileWatcher/FileWatcher.h"

namespace Aho {
	class ResourceLayer : public Layer {
	public:
		ResourceLayer();
		virtual ~ResourceLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		FileWatcher m_FileWatcher;
	};
}