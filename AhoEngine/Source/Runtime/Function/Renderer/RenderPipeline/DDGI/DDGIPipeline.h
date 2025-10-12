#pragma once

#include "../RenderPipeline.h"
#include <glm/glm.hpp>

//Reference: https://github.com/NVIDIAGameWorks/RTXGI-DDGI
namespace Aho {
    static constexpr uint32_t PROBE_IRRADIANCE_TEXELS = 14;
    static constexpr uint32_t PROBE_DISTANCE_TEXELS = 14;
    struct DDGIVolumeDesc {
        glm::vec3 origin = {-150, -5.0, -150};                 
        float probeSpacing = 8.0;               
        glm::ivec3 probeCounts = { 18, 18, 18 };   

        // --- Ray Tracing ---
        int raysPerProbe = 256;                
        float probeMaxRayDistance = 100.0f;    

        // --- Irradiance ---
        float irradianceGamma = 5.0f;           
        float hysteresis = 0.98f;               

        // --- offset to avoid self-shadowing ---
        float viewBias = 0.1f;                 
        float normalBias = 0.1f;                

        // --- debug ---
        bool showProbes = false;               
    };

    class VertexArray;
    class Shader;
    class DDGIPipeline : public RenderPipeline {
    public:
        ~DDGIPipeline();
        DDGIPipeline() { Initialize(); }
        virtual void Initialize() override final;
        virtual void Execute() override final;
        virtual bool Resize(uint32_t width, uint32_t height) const override final;
		bool ShowProbeVisual() const { return m_Desc.showProbes; }
        _Texture* GetProbeIrradianceTex() const;
		_Texture* GetProbeDistanceTex() const;
        void SetUniforms(Shader* shader, uint32_t slot) const;
    private:
		glm::mat3 m_RandomRotation{ 1.0f };
    private:
        std::shared_ptr<VertexArray> m_ProbeVAO{ nullptr };
        _Texture* m_ProbeVisualTex{ nullptr };
        _Texture* m_ProbeDepthTex{ nullptr };
    private:
        std::unique_ptr<RenderPassBase> m_CompositePass{ nullptr };
        std::unique_ptr<RenderPassBase> m_ProbeRayTracePass{ nullptr };
        std::unique_ptr<RenderPassBase> m_ProbeUpdateIrradiancePass{ nullptr };
		std::unique_ptr<RenderPassBase> m_ProbeUpdateDistancePass{ nullptr };
        std::unique_ptr<RenderPassBase> m_ProbeVisualizationPass{ nullptr };
    private:
        std::shared_ptr<_Texture> m_DDGIProbeIrradiance;
        std::shared_ptr<_Texture> m_DDGIProbeDistance;
        std::shared_ptr<_Texture> m_DDGIProbeRadianceDistance;
    private:
        DDGIVolumeDesc m_Desc;
    };
}