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
        CameraManager(float editorFOV, float gameFOV, float aspectRatio, float nearPlane, float farPlane)
            : m_CurrentMode(CameraMode::Editor),
            m_EditorCamera(std::make_shared<EditorCamera>(editorFOV, aspectRatio, nearPlane, farPlane)),
            m_RuntimeCamera(std::make_shared<RuntimeCamera>(gameFOV, aspectRatio, nearPlane, farPlane)) {
        }

        void SetCameraMode(CameraMode mode) {
            m_CurrentMode = mode;
        }

        CameraMode GetCameraMode() const {
            return m_CurrentMode;
        }

        std::shared_ptr<Camera> GetCurrentCamera() const {
            switch (m_CurrentMode) {
                case CameraMode::Editor:
                    return m_EditorCamera;
                case CameraMode::Runtime:
                    return m_RuntimeCamera;
            }
        }

        void Update(float deltaTime) {
            switch (m_CurrentMode) {
            case CameraMode::Editor:
                m_EditorCamera->Update(deltaTime);
                break;
            case CameraMode::Runtime:
                m_RuntimeCamera->Update(deltaTime);
                break;
            }
        }

    private:
        CameraMode m_CurrentMode = Editor;
        std::shared_ptr<EditorCamera> m_EditorCamera;
        std::shared_ptr<RuntimeCamera> m_RuntimeCamera;
    };
} // namespace Aho
