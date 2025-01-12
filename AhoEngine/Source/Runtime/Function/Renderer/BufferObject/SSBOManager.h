#pragma once
#include "SSBO.h"
#include <unordered_map>
#include <memory>

namespace Aho {
    class SSBOManager {
    public:
        template <typename T>
        inline static void RegisterSSBO(uint32_t bindingPoint, int64_t maxElements, bool staticDraw = false) {
            if (!m_SSBOs.contains(bindingPoint)) {
                m_SSBOs[bindingPoint] = std::make_unique<SSBO<T>>(bindingPoint, maxElements, staticDraw);
            }
            else {
                AHO_CORE_ERROR("Registering SSBO failed: binding point {} is already occupied", bindingPoint);
            }
        }

        
        template <typename T>
        inline static void UpdateSSBOData(uint32_t bindingPoint, const std::vector<T>& data, size_t offset = 0) {
            auto it = m_SSBOs.find(bindingPoint);
            if (it != m_SSBOs.end()) {
                auto ubo = static_cast<SSBO<T>*>(it->second.get()); // dynamic cast for safefy
                if (ubo) {
                    ubo->SetSubData(data, offset);
                }
                else {
                    AHO_CORE_ERROR("SSBO type mismatch for binding point: {}", std::to_string(bindingPoint));
                }
            }
            else {
                AHO_CORE_ERROR("Binding point {} does not exist", bindingPoint);
            }
        }

        template <typename T>
        inline static void GetSubData(uint32_t bindingPoint, std::vector<T>& data, size_t offset) {
            auto it = m_SSBOs.find(bindingPoint);
            if (it != m_SSBOs.end()) {
                auto ubo = static_cast<SSBO<T>*>(it->second.get()); // dynamic cast for safefy
                if (ubo) {
                    ubo->GetSubData(data, offset);
                }
                else {
                    AHO_CORE_ERROR("SSBO type mismatch for binding point: {}", std::to_string(bindingPoint));
                }
            }
            else {
                AHO_CORE_ERROR("Binding point {} does not exist", bindingPoint);
            }
        }

    private:
        inline static std::unordered_map<uint32_t, std::unique_ptr<SSBOBase>> m_SSBOs;
    };
}