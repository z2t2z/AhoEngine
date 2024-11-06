#include "Ahopch.h"
#include "Renderer.h"
#include "VertexArrayr.h"
#include "RenderCommand.h"

namespace Aho {
	uint32_t GlobalState::g_SelectedEntityID = 1'000'000'007;
	
	bool GlobalState::g_ShowDebug = false;
	
	bool GlobalState::g_IsEntityIDValid = false;

	std::shared_ptr<RenderData> GlobalState::g_SelectedData = nullptr;

	Renderer::Renderer() {

	}
}