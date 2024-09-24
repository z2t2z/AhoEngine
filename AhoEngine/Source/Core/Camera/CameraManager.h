#pragma once

#include "Core/Input/Input.h"

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
            glm::vec3 movement(0.0f);
            if (Input::IsKeyPressed(AHO_KEY_W)) {
                movement.z += 1.0f;
            }
            if (Input::IsKeyPressed(AHO_KEY_S)) {
                movement.z -= 1.0f;
            }
            if (Input::IsKeyPressed(AHO_KEY_A)) {
                movement.x -= 1.0f;
            }
            if (Input::IsKeyPressed(AHO_KEY_D)) {
                movement.x += 1.0f;
                
            }
            GetMainEditorCamera()->Update(deltaTime, movement);
        }

        std::shared_ptr<Camera> GetMainEditorCamera() { AHO_CORE_ASSERT(!m_Cameras.empty()); return m_Cameras[0]; }

    private:
        std::vector<std::shared_ptr<Camera>> m_Cameras;
    };
} // namespace Aho
