#pragma once

#include "Core/Input/Input.h"

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

    class CameraManager {
    public:
        CameraManager() {
            std::shared_ptr<EditorCamera> cam = std::make_shared<EditorCamera>(45.0f, 16.0f / 9.0f, 0.01f, 1000.0f);
            m_Cameras.push_back(cam);
        }
        ~CameraManager() = default;

        int AddCamera(std::shared_ptr<Camera> cam) { m_Cameras.push_back(cam); return m_Cameras.size(); }
        
        void Update(float deltaTime) {
            if (!Input::IsMouseButtonPressed(AHO_MOUSE_BUTTON_RIGHT)) {
                return;
            }
            // Handle rotation
            auto [mouseX, mouseY] = Input::GetMousePosition();
            //AHO_TRACE("{},{}", mouseX, mouseY);
            glm::vec2 delta = 0.002f * glm::vec2(mouseX - m_LastMouseX, mouseY - m_LastMouseY);
            std::swap(mouseX, m_LastMouseX);
            std::swap(mouseY, m_LastMouseY);
            
            auto cam = GetMainEditorCamera();
            if (delta.x != 0 || delta.y != 0) {
                float pitchDelta = delta.y * cam->GetRotationSpeed();
                float yawDelta = delta.x * cam->GetRotationSpeed();

                glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, cam->GetRight()), glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
                //cam->SetForwardRotation(q);
            }

            // Handle WASD movement
            glm::vec3 movement(0.0f);
            if (Input::IsKeyPressed(AHO_KEY_W)) {
                movement.z -= 1.0f;
            }
            if (Input::IsKeyPressed(AHO_KEY_S)) {
                movement.z += 1.0f;
            }
            if (Input::IsKeyPressed(AHO_KEY_A)) {
                movement.x -= 1.0f;
            }
            if (Input::IsKeyPressed(AHO_KEY_D)) {
                movement.x += 1.0f;
                
            }
            GetMainEditorCamera()->Update(deltaTime, movement);

            auto pos = GetMainEditorCamera()->GetPosition();
            auto front = GetMainEditorCamera()->GetFront();
            auto right = GetMainEditorCamera()->GetRight();
            AHO_TRACE("Position: {}, {}, {}", pos.x, pos.y, pos.z);
            AHO_TRACE("Front: {}, {}, {}", front.x, front.y, front.z);
            AHO_TRACE("Right: {}, {}, {}", right.x, right.y, right.z);
        }

        std::shared_ptr<Camera> GetMainEditorCamera() { AHO_CORE_ASSERT(!m_Cameras.empty()); return m_Cameras[0]; }

    private:
        float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
        std::vector<std::shared_ptr<Camera>> m_Cameras;
    };
} // namespace Aho
