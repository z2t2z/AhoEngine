#pragma once

#include "Runtime/Core/Input/Input.h"

#include "RuntimeCamera.h"
#include "EditorCamera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory>

namespace Aho {
    enum class CameraMode {
        Editor,
        Runtime
    };
       
    // TODO: Should change name to `CameraController` but whatever
    class CameraManager {
    public:
        CameraManager() {
            std::shared_ptr<EditorCamera> cam = std::make_shared<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f); // 1000m far plane
            m_Cameras.push_back(cam);
        }
        ~CameraManager() = default;

        size_t AddCamera(std::shared_ptr<Camera> cam) { m_Cameras.push_back(cam); return m_Cameras.size(); }
        
        bool Update(float deltaTime, bool firstClick);

        std::shared_ptr<Camera> GetMainEditorCamera() { AHO_CORE_ASSERT(!m_Cameras.empty()); return m_Cameras[0]; }
        float& GetSensitivity() { return m_Sensitivity; }
        float& GetSpeed() { return m_Speed; }
    private:
        void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);
    
    private:
        float m_LastYaw{ 0.0f };
        float m_LastForward{ 0.0f };
        float m_LastRight{ 0.0f };
        float m_Speed{ 25.0f };
        float m_CurrSpeed{ 0.0f };
        float m_Acceleration{ 20.0f };
    private:
        float m_Sensitivity{ 2.0f };
        float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
        float m_LockedMouseX, m_LockedMouseY;
        std::vector<std::shared_ptr<Camera>> m_Cameras;
    };
} // namespace Aho
