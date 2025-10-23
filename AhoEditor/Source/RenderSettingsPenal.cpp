#include "RenderSettingsPenal.h"


namespace Aho {
	void RenderSettingsPenal::Draw() {
        ImGui::Begin("Render Settings");

        ImGui::Checkbox("Enable Shadows", &m_Settings.enableShadows);
        if (m_Settings.enableShadows) {
            ShowShadowSettingsPanel();
		}

        ImGui::Checkbox("Enable SSAO", &m_Settings.enableSSAO);
        ImGui::Checkbox("Enable Bloom", &m_Settings.enableBloom);

        ImGui::SliderFloat("Exposure", &m_Settings.exposure, 0.1f, 5.0f);
        ImGui::SliderFloat("Gamma", &m_Settings.gamma, 1.0f, 3.0f);

        ImGui::End();
	}

    void RenderSettingsPenal::ShowShadowSettingsPanel() {
        ImGui::Begin("Shadow Settings");

        const char* shadowTypes[] = { "Hard Shadow", "PCF", "PCSS", "Ray Trace" };
        int currentType = static_cast<int>(m_ShadowSettings.type);
        if (ImGui::Combo("Shadow Type", &currentType, shadowTypes, IM_ARRAYSIZE(shadowTypes))) {
            m_ShadowSettings.type = static_cast<ShadowType>(currentType);
        }

        switch (m_ShadowSettings.type) {
            case ShadowType::PCF:
                ImGui::SliderInt("PCF Kernel Size", &m_ShadowSettings.pcfKernelSize, 1, 15);
                break;
            case ShadowType::PCSS:
                ImGui::SliderFloat("Light Size", &m_ShadowSettings.pcssLightSize, 0.01f, 0.2f);
                ImGui::SliderFloat("Min Penumbra", &m_ShadowSettings.pcssMinPenumbra, 0.0f, 0.5f);
                ImGui::SliderFloat("Max Penumbra", &m_ShadowSettings.pcssMaxPenumbra, 0.5f, 2.0f);
                break;
            case ShadowType::HardShadow:
            case ShadowType::RayTrace:
            case ShadowType::None:
                break;
        }

        ImGui::End();
    }

}