#pragma once

#include <memory>

namespace Aho {
    enum class ShaderFeature : uint32_t {
        None = 0,
        ENABLE_INFINITE_LIGHT = 1 << 0,
        USE_UNIFORM_INFINITE_LIGHT = 1 << 1,
        USE_ENV_MAP_INFINITE_LIGHT = 1 << 2,
    };

    inline ShaderFeature operator|(ShaderFeature a, ShaderFeature b) {
        return static_cast<ShaderFeature>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline bool operator&(ShaderFeature a, ShaderFeature b) {
        return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
    }

}
