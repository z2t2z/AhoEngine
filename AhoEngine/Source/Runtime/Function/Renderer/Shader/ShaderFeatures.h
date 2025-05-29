#pragma once

#include <string>

namespace Aho {
    enum class ShaderFeature : uint32_t {
        NONE = 0,
        FEATURE_IBL = 1 << 0,
        FEATURE_SKY_ATMOSPHERIC = 1 << 1,
        //USE_ENV_MAP_INFINITE_LIGHT = 1 << 2,
    };

    inline ShaderFeature operator|(ShaderFeature a, ShaderFeature b) {
        return static_cast<ShaderFeature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
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
        Deferred,
        PathTracing,
    };
}
