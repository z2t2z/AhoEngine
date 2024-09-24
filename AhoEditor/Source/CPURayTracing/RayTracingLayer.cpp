#include "RayTracingLayer.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Aho {
	RayTracingLayer::RayTracingLayer() {
	}

	void RayTracingLayer::OnAttach() {
		AHO_TRACE("Attaching raytracing layer");
		//m_Camera = std::make_unique<EditorCamera>(45, 16.0f / 9.0f, 0.01f, 100.0f);
	}

	void RayTracingLayer::OnUpdate(float deltaTime) {

	}

	void RayTracingLayer::OnImGuiRender() {
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render")) {
			//Render();
		}

		//ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		//if (ImGui::Button("Reset"))
			//m_Renderer.ResetFrameIndex();

		ImGui::End();

		ImGui::Begin("Scene");
		//for (size_t i = 0; i < m_Scene.Spheres.size(); i++) {
		//	ImGui::PushID(i);

		//	Sphere& sphere = m_Scene.Spheres[i];
		//	ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
		//	ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
		//	ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);

		//	ImGui::Separator();

		//	ImGui::PopID();
		//}

		//for (size_t i = 0; i < m_Scene.Materials.size(); i++) {
		//	ImGui::PushID(i);

		//	Material& material = m_Scene.Materials[i];
		//	ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
		//	ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
		//	ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);
		//	ImGui::ColorEdit3("Emission Color", glm::value_ptr(material.EmissionColor));
		//	ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);

		//	ImGui::Separator();

		//	ImGui::PopID();
		//}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		//auto image = m_Renderer.GetFinalImage();
		//if (image)
		//	ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
		//		ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();
	}
} // namespace Aho