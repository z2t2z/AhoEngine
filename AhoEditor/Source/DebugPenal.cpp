#include "DebugPenal.h"
#include "IamAho.h"
#include "Runtime/Function/Renderer/BufferObject/SSBOManager.h"
#include "Runtime/Function/Renderer/DisneyPrincipled.h"

namespace Aho {
	DebugPenal::DebugPenal() {

	}

	void DebugPenal::Draw() {
		BVHControl();
	}

	bool DebugPenal::SunDirControl() {
		auto sundir = m_SkyPipeline->GetSunDir();
		ImGui::Begin("Temp Sky Control");

		static glm::vec3 sunRotation(45.0f, 0.0f, 0.0f);
		bool changed = ImGui::DragFloat3("SunRotation", &sunRotation[0], 0.1f);

		float theta = glm::radians(sunRotation.x), phi = glm::radians(sunRotation.y);
		glm::vec3 sunDir = normalize(glm::vec3(glm::sin(theta) * glm::cos(phi), glm::cos(theta), glm::sin(theta) * glm::sin(phi)));
		//std::string text = std::to_string(sunDir.x) + " " + std::to_string(sunDir.y) + " " + std::to_string(sunDir.z);
		//ImGui::Text(text.c_str());
		m_SkyPipeline->SetSunDir(sunDir);
		m_ShadingPipeline->SetSunDir(sunDir);
		ImGui::End();
		return changed;
	}

	void DebugPenal::BVHControl() {
		ImGui::Begin("Temp bvh control");
		auto entityManager = m_LevelLayer->GetCurrentLevel()->GetEntityManager();
		auto view = entityManager->GetView<BVHComponent, TransformComponent>();
		int find = -1;

		if (ImGui::Button("Update BVH")) {
			bool dirty = false;
			view.each(
				[&dirty](auto entity, BVHComponent& bc, TransformComponent& tc) {
					if (tc.dirty) {
						tc.dirty = false;
						dirty = true;
						for (BVHi* bvh : bc.bvhs) {
							bvh->ApplyTransform(tc.GetTransform());
						}
					}
				});


			if (dirty) {
				BVHi& alts = m_LevelLayer->GetCurrentLevel()->GetTLAS();
				alts.UpdateTLAS();
				m_PtPipeline->UpdateSSBO(m_LevelLayer->GetCurrentLevel());
			}
		}

		if (ImGui::Button("MagicButton")) {
			GetSSBOData();
		}

		// BVH Intersection test
		//if (ImGui::Button("GetData")) {
		//	std::vector<BVHNodei> data(1);
		//	SSBOManager::GetSubData<BVHNodei>(0, data, 0);
		//}
		//view.each(
		//	[&](auto entity, BVHComponent& bc, TransformComponent& tc) {
		//		ImGui::Text("EntityID: %d", static_cast<uint32_t>(entity));
		//		std::string showName = "Update BVH:" + std::to_string(static_cast<uint32_t>(entity));
		//		if (ImGui::Button(showName.c_str())) {
		//			//ScopedTimer timer(std::to_string(static_cast<uint32_t>(entity)));
		//			bc.bvh.ApplyTransform(tc.GetTransform());
		//		}
		//		
		//		if (find == -1 && m_ShouldPickObject) {
		//			auto cam = m_CameraManager->GetMainEditorCamera();
		//			{
		//				ScopedTimer timer("Idx Intersecting test");
		//				if (bc.bvh.Intersect(m_Ray)) {
		//					find = static_cast<uint32_t>(entity);
		//				}
		//			}

		//			// testing ptr version bvh
		//			//if (!intersectionResult) {
		//			//	ScopedTimer timer("PTR Intersecting test");
		//			//	intersectionResult = BVH::GetIntersection(m_Ray, root.get());
		//			//}
		//		
		//		}

		//		ImGui::Separator();
		//	});

		//if (m_ShouldPickObject && Input::IsKeyPressed(AHO_KEY_LEFT_CONTROL)) {
		//	BVHi& alts = m_LevelLayer->GetCurrentLevel()->GetTLAS();
		//	ScopedTimer timer("Intersecting test");
		//	if (alts.Intersect(m_Ray)) {
		//		find = 1;
		//	}
		//}


		//if (find != -1) {
		//	AHO_CORE_TRACE("Intersecting {}", find);
		//}
		//m_ShouldPickObject = false;
		ImGui::End();
	}

	void DebugPenal::GetSSBOData() {
		std::vector<TextureHandles> data(4);
		SSBOManager::GetSubData<TextureHandles>(5, data, 0);
		AHO_CORE_INFO("{}", data.size());
	}

}