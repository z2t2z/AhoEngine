#include "DebugPenal.h"
#include "IamAho.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"

namespace Aho {
	DebugPenal::DebugPenal() {

	}

	void DebugPenal::Draw() {
		//BVHControl();
		//SunDirControl();
	}

	bool DebugPenal::SunDirControl() {
		auto sundir = m_SkyPipeline->GetSunDir();
		ImGui::Begin("Temp Sky Control");

		auto entityManager = g_RuntimeGlobalCtx.m_EntityManager;
		auto view = entityManager->GetView<AtmosphereParametersComponent, LightComponent>();
		bool changed = false;
		static glm::vec3 sunDir = { 60.0f, -90.0f, 0.0f }; // In angles
		view.each(
			[&](auto entity, AtmosphereParametersComponent& apc, LightComponent& dlc) {
				changed |= ImGui::DragFloat3("SunRotation", &sunDir[0], 0.01f);
				if (changed) {
					float theta = glm::radians(sunDir.x);
					float phi = glm::radians(sunDir.y);
					std::shared_ptr<DirectionalLight> light = std::static_pointer_cast<DirectionalLight>(dlc.light);
					light->SetDirection(glm::vec3(glm::sin(theta) * glm::cos(phi), glm::cos(theta), glm::sin(theta) * glm::sin(phi)));
				}
			});
		ImGui::End();
		return changed;
	}

	void DebugPenal::BVHControl() {
		ImGui::Begin("Temp bvh control");
		if (ImGui::Button("Get SSBO Data")) {
			GetSSBOData();
		}
		ImGui::End();
	}

	void DebugPenal::GetSSBOData() {
		std::vector<BVHNodei> data(47213);
		SSBOManager::GetSubData<BVHNodei>(2, data, 0);

		std::vector<PrimitiveDesc> data1(51267);
		SSBOManager::GetSubData<PrimitiveDesc>(3, data1, 0);


		AHO_CORE_INFO("{}", data.size());
	}

}