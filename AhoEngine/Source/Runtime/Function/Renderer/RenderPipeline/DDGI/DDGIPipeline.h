#pragma once

#include "../RenderPipeline.h"
#include <glm/glm.hpp>

//Reference: https://github.com/NVIDIAGameWorks/RTXGI-DDGI
namespace Aho {
    struct alignas(16) DDGIVolumeDesc {
        glm::vec3 origin = {};              // volume ��㣨world-space��
        float probeSpacing = {};        // ÿ��̽��֮��ļ��
        glm::ivec3 probeCounts = { -1, -1, -1 }; // ÿ���ϵ� probe ����

        // --- Ray Tracing ---
        int probeNumRays = 256;             // ÿ��̽�뷢�����������
        float probeMaxRayDistance = 100.0f; // ÿ�� ray �����׷�پ���

        // --- Irradiance ---
        int probeTexelPerProbe = 6;         // ÿ�� probe �� oct map �ߴ磨N��N��
        float irradianceGamma = 5.0f;       // �洢����ʱ�� gamma
        float hysteresis = 0.97f;           // ʱ���ں�ϵ����Խ��Խ�ȶ���

        // --- offset to avoid self-shadowing ---
        float viewBias = 0.1f;              // ���ӽǷ��� ray ʱ��ƫ��
        float normalBias = 0.1f;            // �ӷ��߷������ irradiance ʱƫ��

        // --- debug ---
        bool showProbes = false;            // �Ƿ�����̽����ӻ�
        bool debugIrradiance = false;       // �Ƿ���Ⱦ irradiance atlas debug ͼ
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