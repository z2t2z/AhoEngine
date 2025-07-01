#pragma once

#include "../RenderPipeline.h"
#include <glm/glm.hpp>

//Reference: https://github.com/NVIDIAGameWorks/RTXGI-DDGI
namespace Aho {
    struct alignas(16) DDGIVolumeDesc {
        glm::vec3 origin = {};              // volume 起点（world-space）
        float probeSpacing = {};        // 每个探针之间的间距
        glm::ivec3 probeCounts = { -1, -1, -1 }; // 每轴上的 probe 数量

        // --- Ray Tracing ---
        int probeNumRays = 256;             // 每个探针发射的射线数量
        float probeMaxRayDistance = 100.0f; // 每条 ray 的最大追踪距离

        // --- Irradiance ---
        int probeTexelPerProbe = 6;         // 每个 probe 的 oct map 尺寸（N×N）
        float irradianceGamma = 5.0f;       // 存储编码时的 gamma
        float hysteresis = 0.97f;           // 时域融合系数（越高越稳定）

        // --- offset to avoid self-shadowing ---
        float viewBias = 0.1f;              // 从视角发射 ray 时的偏移
        float normalBias = 0.1f;            // 从法线方向采样 irradiance 时偏移

        // --- debug ---
        bool showProbes = false;            // 是否启用探针可视化
        bool debugIrradiance = false;       // 是否渲染 irradiance atlas debug 图
    };

    class VertexArray;
    class DDGIPipeline : public RenderPipeline {
    public:
        ~DDGIPipeline();
        DDGIPipeline() { Initialize(); }
        virtual void Initialize() override final;
        virtual void Execute() override final;
        virtual bool Resize(uint32_t width, uint32_t height) const override final;
    private:
        //void ClearProbes();
        //void RayTrace(uint32_t tlas, uint32_t frameIndex);
        //void Blend();
    private:
        std::shared_ptr<VertexArray> m_ProbeVAO{ nullptr };
        _Texture* m_ProbeVisualTex{ nullptr };
        _Texture* m_ProbeDepthTex{ nullptr };

    private:
        std::unique_ptr<RenderPassBase> m_CompositePass{ nullptr };
        std::unique_ptr<RenderPassBase> m_ProbeRayTracePass{ nullptr };
        std::unique_ptr<RenderPassBase> m_ProbeVisualizationPass{ nullptr };
    private:
        DDGIVolumeDesc m_Desc;
        _Texture* m_ProbeIrradianceTex = 0;
        _Texture* m_ProbeDistanceTex = 0;
        _Texture* m_ProbeRayDataTex = 0;
    };
}