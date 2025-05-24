#include "DebugPenal.h"
#include "IamAho.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"

namespace Aho {
	DebugPenal::DebugPenal() {

	}

	void DebugPenal::Draw() {
		BVHControl();
		SunDirControl();
	}

	bool DebugPenal::SunDirControl() {
		auto sundir = m_SkyPipeline->GetSunDir();
		ImGui::Begin("Temp Sky Control");

		auto entityManager = g_RuntimeGlobalCtx.m_EntityManager;
		auto view = entityManager->GetView<AtmosphereParametersComponent, DistantLightComponent>();
		bool changed = false;
		static glm::vec3 sunDir = { 60.0f, -90.0f, 0.0f }; // In angles
		view.each(
			[&](auto entity, AtmosphereParametersComponent& apc, DistantLightComponent& dlc) {
				changed |= ImGui::DragFloat3("SunRotation", &sunDir[0], 0.01f);
				if (changed) {
					float theta = glm::radians(sunDir.x);
					float phi = glm::radians(sunDir.y);
					dlc.LightDir = glm::vec3(glm::sin(theta) * glm::cos(phi), glm::cos(theta), glm::sin(theta) * glm::sin(phi));
				}
			});
		ImGui::End();
		return changed;
	}

	void DebugPenal::BVHControl() {
		return;
		ImGui::Begin("Temp bvh control");
		ImGui::End();
	}

	void DebugPenal::GetSSBOData() {
		std::vector<MaterialDescriptor> data(4);
		SSBOManager::GetSubData<MaterialDescriptor>(5, data, 0);
		AHO_CORE_INFO("{}", data.size());
	}

}