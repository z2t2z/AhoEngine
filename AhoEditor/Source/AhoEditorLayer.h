#pragma once

#include "IamAho.h"
#include "EditorUI/LevelHierarchyPanel/LevelHierarchyPanel.h"

namespace Aho {
	class AhoEditorLayer : public Layer {
	public:
		AhoEditorLayer(LevelLayer* levellayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager);
		virtual ~AhoEditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		bool OnFileChanged(FileChangedEvent& event);
		void DrawContentBrowserPanel();
		void DrawPropertiesPanel();
		void DrawSceneHierarchyPanel();
		void DrawViewport();
		void DrawLightIcons();
		void DrawToolBar();
	private:
		std::filesystem::path m_FolderPath;
		std::filesystem::path m_CurrentPath;
	private:
		bool m_Selected{ false };
		bool m_DrawDepthMap{ false };
		bool m_PickingPass{ false };
		bool m_IsViewportFocused{ false };
		bool m_CursorInViewport{ false };
		bool m_PickObject{ false };
		bool m_BlockClickingEvent{ false };
		uint32_t m_PickData{ 998244353u };
		Entity m_SelectedObject;
		float m_DeltaTime{ 0.0f };
	private:
		//std::unique_ptr<Texture2D> m_LightIcon; ??
		std::shared_ptr<Texture2D> m_LightIcon{ nullptr };
		std::shared_ptr<Texture2D> m_PlusIcon{ nullptr };
		std::shared_ptr<Texture2D> m_TranslationIcon{ nullptr };
		std::shared_ptr<Texture2D> m_RotationIcon{ nullptr };
		std::shared_ptr<Texture2D> m_ScaleIcon{ nullptr };
	private:
		Renderer* m_Renderer{ nullptr };
		LevelLayer* m_LevelLayer{ nullptr }; 
		FileWatcher m_FileWatcher;
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	};
}