#include "Ahopch.h"
#include "GlobalContext.h"

#include "Runtime/Core/Log/Log.h"
#include "Runtime/Core/Input/Input.h"
#include "Runtime/Core/Parallel.h";
#include "Runtime/Function/Renderer/Renderer.h"
#include "Runtime/Function/Level/Level.h"
#include "Runtime/Function/Level/EcS/EntityManager.h"
#include "Runtime/Function/Renderer/IBL/IBLManager.h"
#include "Runtime/Resource/Asset/AssetManager.h"
#include "Runtime/Resource/ResourceManager.h"

namespace Aho {
	RuntimeGlobalContext g_RuntimeGlobalCtx;

	void RuntimeGlobalContext::InitializeSystems(const std::string& configFilePath) {
		m_Resourcemanager	= std::make_shared<ResourceManager>();
		m_AssetManager		= std::make_shared<AssetManager>();
		m_EntityManager		= std::make_shared<EntityManager>();
		m_Renderer			= std::make_shared<Renderer>();
		m_Renderer->Initialize();
		m_IBLManager     	= std::make_shared<IBLManager>();
		m_ParallelExecutor  = std::make_unique<ParallelExecutor>();
	}

	void RuntimeGlobalContext::ShutdownSystems() {
		m_Logger.reset();
		m_Renderer.reset();
		m_InputSystem.reset();
		m_FileSystem.reset();
		m_AssetManager.reset();
		m_LevelManager.reset();
		m_WindowSystem.reset();
		m_Resourcemanager.reset();
	}
}