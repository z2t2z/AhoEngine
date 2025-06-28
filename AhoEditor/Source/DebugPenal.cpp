#include "DebugPenal.h"
#include "IamAho.h"
#include "EditorUI/ImGuiHelpers.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"

namespace Aho {
	DebugPenal::DebugPenal() {
	}

	void DebugPenal::Draw() {
		static bool drawDebugOverlay = true;
		auto passes = g_RuntimeGlobalCtx.m_Renderer->GetAllRenderPasses();
		std::vector<std::pair<std::string, float>> infos(passes.size());
		for (size_t i = 0; i < passes.size(); i++) {
			infos[i] = std::make_pair(passes[i]->GetPassName(), passes[i]->GetFrameTime());
		}
		ImGuiHelpers::DrawStatisticOverlay(&drawDebugOverlay, infos);
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