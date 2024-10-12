#include "ViewportPanel.h"
#include <imgui.h>

namespace Aho {
	bool ViewportPanel::DrawPanel() {
		ImGui::Begin("Viewport");
		auto [width, height] = ImGui::GetWindowSize();
		m_FBO->Bind();
		auto spec = m_FBO->GetSpecification();
		// TODO: minimum height
		if (spec.Width != width || spec.Height != height/* - ImGui::GetFrameHeight() */) {
			m_FBO->Resize(width, height/* - ImGui::GetFrameHeight() */);
			m_CameraManager->GetMainEditorCamera()->SetProjection(45, width / height, 0.1f, 1000.0f);  // TODO: camera settings
		}
		uint32_t RenderResult = m_FBO->GetLastColorAttachment();
		ImGui::Image((void*)(uintptr_t)RenderResult, ImVec2{ width, height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		auto [MouseX, MouseY] = ImGui::GetMousePos();
		auto [windowPosX, windowPosY] = ImGui::GetWindowPos();
		int x = MouseX - windowPosX, y = MouseY - windowPosY - ImGui::GetFrameHeight(); // mouse position in the current window
		y = spec.Height - y;
		uint32_t readData = 0;
		if (x >= 0 && y >= 0 && x < width && y < height) {
			m_CursorInViewport = true;
			readData = m_FBO->ReadPixel(0, x, y);
		}
		else {
			m_CursorInViewport = false;
		}
		m_FBO->Unbind();
		if (readData) {
			//AHO_TRACE("{}", readData);
		}

		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM");
			if (payload) {
				const char* droppedData = static_cast<const char*>(payload->Data);
				std::string droppedString(droppedData, payload->DataSize);
				AHO_TRACE("Payload accepted! {}", droppedString);
				std::shared_ptr<AssetImportedEvent> event = std::make_shared<AssetImportedEvent>(droppedString);
				AHO_CORE_WARN("Pushing a AssetImportedEvent!");
				m_EventManager->PushBack(event);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();
		return m_CursorInViewport;
	}
}