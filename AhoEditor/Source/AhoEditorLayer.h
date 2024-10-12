#pragma once

#include "IamAho.h"
#include "EditorUI/LevelHierarchyPanel/LevelHierarchyPanel.h"
#include "EditorUI/ViewportPanel/ViewportPanel.h"

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
		void DrawEditorPanel();
		void DrawContentBrowserPanel();
		void DrawSceneHierarchyPanel();
		void DrawViewport();
	private:
		std::filesystem::path m_FolderPath;
		std::filesystem::path m_CurrentPath;
	private:
		bool m_IsViewportFocused{ false };
		bool m_CursorInViewport{ false };
		bool m_PickObject{ false };
		bool m_IsDragging{ false };
		bool m_BlockClickingEvent{ false };
		uint32_t m_PickData{ 998244353u };
		Entity m_SelectedObject;
		float m_DeltaTime{ 0.0f };
	private:
		std::shared_ptr<Framebuffer> m_FBO{ nullptr };
		FileWatcher m_FileWatcher;
		LevelLayer* m_LevelLayer{ nullptr };
		Renderer* m_Renderer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	private:
		ViewportPanel* m_ViewportPanel{ nullptr };
	};
}