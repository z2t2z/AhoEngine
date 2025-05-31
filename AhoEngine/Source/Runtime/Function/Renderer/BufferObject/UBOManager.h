#pragma once
#include "UBOStructs.h"
#include "UBO.h"
#include <unordered_map>
#include <memory>

namespace Aho {
    class UBOManager {
    public:
        template <typename T>
        static void RegisterUBO(uint32_t bindingPoint) {
            if (!m_UBOs.contains(bindingPoint)) {
                m_UBOs[bindingPoint] = std::make_unique<UBO<T>>(bindingPoint);
            }
            else {
                AHO_CORE_ERROR("Binding point {} is already occupied", bindingPoint);
            }
        }

        template <typename T>
        static void UpdateUBOData(uint32_t bindingPoint, const T& data, size_t offset = 0) {
            auto it = m_UBOs.find(bindingPoint);
            if (it != m_UBOs.end()) {
                auto ubo = static_cast<UBO<T>*>(it->second.get());
                if (ubo) {
                    ubo->SetData(data, offset);
                }
                else {
                    throw std::runtime_error("UBO type mismatch for binding point " + std::to_string(bindingPoint));
                }
            }
            else {
                AHO_CORE_ERROR("Binding point {} does not exist", bindingPoint);
            }
        }

    private:
        static std::unordered_map<uint32_t, std::unique_ptr<UBOBase>> m_UBOs;
    };
}