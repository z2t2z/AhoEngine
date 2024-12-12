#pragma once

#include "IamAho.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace Aho {
	class AhoEditorLayer : public Layer {
	public:
		AhoEditorLayer(LevelLayer* levellayer, ResourceLayer* resourceLayer, EventManager* eventManager, Renderer* renderer, const std::shared_ptr<CameraManager>& cameraManager);
		virtual ~AhoEditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
	private:
		bool OnFileChanged(FileChangedEvent& event);
	private:
		void DrawMaterialPanel();
		void DrawContentBrowserPanel();
		void DrawPropertiesPanel();
		void DrawSceneHierarchyPanel();
		void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
		void DrawViewport();
		void DrawLightIcons();
		void DrawToolBarOverlay();
		void DrawToolBarAddObjectBtn();
		void DrawToolBarRenderModeSelectionBtn();
		void DrawManipulationToolBar();
		void DrawGizmo();
		template<typename T>
		void DrawNode(const T& node);
		void DrawCircle(const ImVec2& center, float radius);
	private:
		void TryGetDragDropTarget();
		std::shared_ptr<Texture2D> TryGetDragDropTargetTexture();
	private:
		uint32_t m_ViewportWidth;
		uint32_t m_ViewportHeight;
		std::string m_DroppedString;
		bool m_StaticMesh{ true };
	private:
		std::filesystem::path m_FolderPath;
		std::filesystem::path m_AssetPath;
		std::filesystem::path m_CurrentPath;
	private:
		bool m_Selected{ false };
		bool m_DrawDepthMap{ false };
		bool m_PickingPass{ false };
		bool m_IsViewportFocused{ false };
		bool m_CursorLocked{ false };
		bool m_IsCursorInViewport{ false };
		bool m_ShouldPickObject{ false };
		bool m_IsClickingEventBlocked{ false };
		uint32_t m_PickPixelData{ 998244353u }; // TODO, try using std::optional?
		Entity m_SelectedObject;
		float m_DeltaTime{ 0.0f };
	private:
		//std::unique_ptr<Texture2D> m_LightIcon; ??
		std::shared_ptr<Texture2D> m_CursorIcon{ nullptr };
		std::shared_ptr<Texture2D> m_LightIcon{ nullptr };
		std::shared_ptr<Texture2D> m_AddIcon{ nullptr };
		std::shared_ptr<Texture2D> m_TranslationIcon{ nullptr };
		std::shared_ptr<Texture2D> m_RotationIcon{ nullptr };
		std::shared_ptr<Texture2D> m_ScaleIcon{ nullptr };
		std::shared_ptr<Texture2D> m_BackIcon{ nullptr };
	private:
		LevelLayer* m_LevelLayer{ nullptr }; 
		ResourceLayer* m_ResourceLayer{ nullptr };
		Renderer* m_Renderer{ nullptr };
		EventManager* m_EventManager{ nullptr };
		std::shared_ptr<CameraManager> m_CameraManager;
	};
}