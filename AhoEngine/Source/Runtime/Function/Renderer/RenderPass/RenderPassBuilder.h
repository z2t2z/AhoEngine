#pragma once

#include "RenderPassBase.h"
#include <string>
#include <memory>
#include <functional>

namespace Aho {
    class _Texture;
    enum class ShaderUsage;

    class RenderPassBuilder {
    public:
        RenderPassBuilder& Name(const std::string& name);
        RenderPassBuilder& Shader(const std::string& shaderPath);
        RenderPassBuilder& Usage(ShaderUsage usage);
        RenderPassBuilder& AttachTarget(const std::shared_ptr<_Texture>& tex);
        RenderPassBuilder& Input(const std::string& name, const std::shared_ptr<_Texture>& tex);
        RenderPassBuilder& Func(std::function<void(RenderPassBase&)> func);
        std::unique_ptr<RenderPassBase> Build();
    private:
        RenderPassConfig m_Cfg;
    };
}