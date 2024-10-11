#pragma once

#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "FileWatcher/FileWatcher.h"

namespace Aho {
	class ResourceLayer : public Layer {
	public:
		ResourceLayer(EventManager* eventManager, AssetManager* m_AssetManager);
		virtual ~ResourceLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		void LoadAssetFromFile(const std::string& path);
		template<typename T>
		void PackEcSData(const T& res);
		template<typename T>
		void PackRenderData(const T& res);
	private:
		AssetManager* m_AssetManager;
		EventManager* m_EventManager;
	};
}