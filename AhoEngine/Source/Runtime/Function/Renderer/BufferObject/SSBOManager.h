#pragma once
#include "SSBO.h"

#include <unordered_map>
#include <memory>

namespace Aho {
    class SSBOManager {
    public:
        // ------- For structs, assume stored in std::vector --------
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


        // ------- For scalars --------
        template <typename T>
        inline static void RegisterScalar(uint32_t bindingPoint) {
            if (!m_SSBOs.contains(bindingPoint)) {
                m_SSBOs[bindingPoint] = std::make_unique<SSBOScalar<T>>(bindingPoint);
            } else {
                AHO_CORE_ERROR("Registering scalar SSBO failed: binding point {} is already occupied", bindingPoint);
            }
        }

        template <typename T>
        inline static void SetScalar(uint32_t bindingPoint, const T& value) {
            auto it = m_SSBOs.find(bindingPoint);
            if (it != m_SSBOs.end()) {
                auto scalar = static_cast<SSBOScalar<T>*>(it->second.get());
                if (scalar) {
                    scalar->Set(value);
                } else {
                    AHO_CORE_ERROR("SSBO type mismatch for binding point: {}", bindingPoint);
                }
            } else {
                AHO_CORE_ERROR("Binding point {} does not exist", bindingPoint);
            }
        }

        template <typename T>
        inline static T GetScalar(uint32_t bindingPoint) {
            auto it = m_SSBOs.find(bindingPoint);
            if (it != m_SSBOs.end()) {
                auto scalar = static_cast<SSBOScalar<T>*>(it->second.get());
                if (scalar) {
                    return scalar->Get();
                } else {
                    AHO_CORE_ERROR("SSBO type mismatch for binding point: {}", bindingPoint);
                }
            } else {
                AHO_CORE_ERROR("Binding point {} does not exist", bindingPoint);
            }
            return {};
        }

        inline static uint32_t GetBufferId(uint32_t bindingPoint) {
            return m_SSBOs.at(bindingPoint)->GetBufferID();
        }

    private:
        inline static std::unordered_map<uint32_t, std::unique_ptr<SSBOBase>> m_SSBOs;
    };
}