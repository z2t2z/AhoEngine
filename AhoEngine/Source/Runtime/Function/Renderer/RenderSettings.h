#pragma once

#include <glm/glm.hpp>

namespace Aho {
    enum class ShadowType {
        None,
        HardShadow,
        PCF,
        PCSS,
        RayTrace
    };

    struct ShadowSettings {
        ShadowType type = ShadowType::HardShadow;

        // PCF params
        int pcfKernelSize = 3;

        // PCSS params
        float pcssLightSize = 0.05f;
        float pcssMinPenumbra = 0.1f;
        float pcssMaxPenumbra = 1.0f;
    };

    struct RenderSettings {
		// Path tracing 
        int TLASStkDepth = 32;
		int BLASStkDepth = 32;

		bool DDGIEnabled = false;

		glm::vec3 uniformSkyColor = glm::vec3(0.1f, 0.1f, 0.1f);

		float environmentIntensity = 1.0f;


        bool enableShadows = true;
        bool enableSSAO = false;
        bool enableBloom = true;
        float exposure = 1.0f;
        float gamma = 2.2f;
    };
}