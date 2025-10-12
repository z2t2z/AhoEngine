#include "Ahopch.h"
#include "IBL.h"
#include "IBLManager.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/RenderCommand.h"

namespace Aho {
	void IBLManager::BindActiveIBL(const Shader* shader) const {
		auto activeIBLEntity = g_RuntimeGlobalCtx.m_IBLManager->GetActiveIBL();
		if (g_RuntimeGlobalCtx.m_EntityManager->HasComponent<IBLComponent>(activeIBLEntity)) {
			auto& iblComp = g_RuntimeGlobalCtx.m_EntityManager->GetComponent<IBLComponent>(activeIBLEntity);
			iblComp.IBL->Bind(shader);
		}
	}

	void IBLManager::BindActiveIBLDeferred(const Shader* shader, uint32_t& slot) const {
		auto activeIBLEntity = g_RuntimeGlobalCtx.m_IBLManager->GetActiveIBL();
		if (g_RuntimeGlobalCtx.m_EntityManager->HasComponent<IBLComponent>(activeIBLEntity)) {
			auto& iblComp = g_RuntimeGlobalCtx.m_EntityManager->GetComponent<IBLComponent>(activeIBLEntity);
			//iblComp.IBL->Bind(shader);
			if (iblComp.EnvTextureSkyBox) {
				shader->Bind();
				shader->SetInt("u_gCubeMap", slot);
				RenderCommand::BindTextureUnit(slot++, iblComp.EnvTextureSkyBox->GetTextureID());
				shader->SetInt("u_gIrradiance", slot);
				RenderCommand::BindTextureUnit(slot++, iblComp.Irradiance->GetTextureID());
				shader->SetInt("u_gPrefilter", slot);
				RenderCommand::BindTextureUnit(slot++, iblComp.Prefilter->GetTextureID());
				shader->SetInt("u_gBRDFLUT", slot);
				RenderCommand::BindTextureUnit(slot++, iblComp.BRDFLUT->GetTextureID());
				shader->SetFloat("u_PrefilterMaxMipLevel", iblComp.Prefilter->GetMipLevels());
			}
		}
	}
}