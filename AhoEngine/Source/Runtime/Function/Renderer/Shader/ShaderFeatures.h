#pragma once

#include <string>

namespace Aho {
    enum class ShaderFeatureShadow : uint32_t {
        NONE = 0,
        FEATURE_SHADOW_SIMPLE_HARD = 1 << 0,
        FEATURE_SHADOW_PCF = 1 << 1,
        FEATURE_SHADOW_PCSS = 1 << 2,
        FEATURE_SHADOW_RAYTRACE = 1 << 3,
    };
    inline ShaderFeatureShadow operator|(ShaderFeatureShadow a, ShaderFeatureShadow b) {
        return static_cast<ShaderFeatureShadow>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
    inline ShaderFeatureShadow& operator|=(ShaderFeatureShadow& a, ShaderFeatureShadow b) {
        a = a | b;
        return a;
	}


    enum class ShaderFeature : uint32_t {
        NONE = 0,
        FEATURE_IBL = 1 << 0,
        FEATURE_SKY_ATMOSPHERIC = 1 << 1,
    };

    inline ShaderFeature operator|(ShaderFeature a, ShaderFeature b) {
        return static_cast<ShaderFeature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline ShaderFeature& operator|=(ShaderFeature& a, ShaderFeature b) {
        a = a | b;
        return a;
    }
    inline ShaderFeature operator^(ShaderFeature a, ShaderFeature b) {
        return static_cast<ShaderFeature>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
    }
    inline ShaderFeature& operator^=(ShaderFeature& a, ShaderFeature b) {
        a = a ^ b;
        return a;
    }
    inline bool operator&(ShaderFeature a, ShaderFeature b) {
        return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
    }

    enum class ShaderType {
        None = 0,
        Normal, // vertex + pixel
        Compute
    };
    enum class ShaderUsage {
        None = 0,
        DistantLightShadowMap,
        DeferredShading,
        PathTracing,
        PathTracingCamRayGen,
        PathTracingPresent,
        GBuffer,
        GenDepthPyramid,
        SSSR,
    };


}
