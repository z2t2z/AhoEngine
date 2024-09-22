#pragma once

#include "Camera.h"
#include "RuntimeCamera.h"
#include "EditorCamera.h"

#include <memory>

namespace Aho {
    enum class CameraMode {
        Editor,
        Runtime
    };

    class CameraManager {
    public:
        CameraManager() {
            std::shared_ptr<EditorCamera> cam = std::make_shared<EditorCamera>(45.0f, 16.0f / 9.0f, 0.01f, 1000.0f);
            m_Cameras.push_back(cam);
        }
        ~CameraManager() = default;

        int AddCamera(std::shared_ptr<Camera> cam) { m_Cameras.push_back(cam); return m_Cameras.size(); }
        
        void Update(float deltaTime) {
            for (auto& cam : m_Cameras) {
                cam->Update(deltaTime);
            }
        }

        std::shared_ptr<Camera> GetMainEditorCamera() { AHO_CORE_ASSERT(!m_Cameras.empty()); return m_Cameras[0]; }
        //std::shared_ptr<Camera> GetMainRuntimeCamera();

    private:
        std::vector<std::shared_ptr<Camera>> m_Cameras;
    };
} // namespace Aho
