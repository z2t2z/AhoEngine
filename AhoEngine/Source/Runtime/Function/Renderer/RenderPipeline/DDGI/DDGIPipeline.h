#pragma once

#include "../RenderPipeline.h"
#include <glm/glm.hpp>

//Reference: https://github.com/NVIDIAGameWorks/RTXGI-DDGI
namespace Aho {
    struct alignas(16) DDGIVolumeDesc {
        glm::vec3 origin = {};              // volume ��㣨world-space��
        glm::vec3 probeSpacing = {};        // ÿ��̽��֮��ļ��
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

    class DDGIPipeline : public RenderPipeline {
    public:
        virtual void Initialize() override final;
    private:
        //void ClearProbes();
        //void RayTrace(uint32_t tlas, uint32_t frameIndex);
        //void Blend();
    private:
        DDGIVolumeDesc m_Desc;
        _Texture* m_ProbeIrradianceTex = 0;
        _Texture* m_ProbeDistanceTex = 0;
        _Texture* m_ProbeRayDataTex = 0;
    };
}