#include "Ahopch.h"
#include "IBL.h"
#include "IBLManager.h"
#include "Runtime/Core/GlobalContext/GlobalContext.h"
#include "Runtime/Function/Renderer/Shader/Shader.h"
#include "Runtime/Function/Level/EcS/Components.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"

namespace Aho {
	void IBLManager::BindActiveIBL(const Shader* shader) const {
		auto activeIBLEntity = g_RuntimeGlobalCtx.m_IBLManager->GetActiveIBL();
		if (g_RuntimeGlobalCtx.m_EntityManager->HasComponent<IBLComponent>(activeIBLEntity)) {
			auto& iblComp = g_RuntimeGlobalCtx.m_EntityManager->GetComponent<IBLComponent>(activeIBLEntity);
			iblComp.IBL->Bind(shader);
		}
	}
}