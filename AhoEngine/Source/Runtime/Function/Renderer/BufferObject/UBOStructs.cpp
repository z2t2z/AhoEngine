#include "Ahopch.h"
#include "UBOStructs.h"
#include "Runtime/Function/Renderer/Lights.h"

namespace Aho {
	void FillDirectionalLightStruct(GPU_DirectionalLight& gpu_light, const std::shared_ptr<DirectionalLight>& light) {
		gpu_light.color = light->GetColor();
		gpu_light.intensity = light->GetIntensity();
		gpu_light.direction = light->GetDirection();
		gpu_light.lightProjView = light->GetProjView();
	}
}
