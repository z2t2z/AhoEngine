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
            std::shared_ptr<EditorCamera> cam = std::make_shared<EditorCamera>(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
            m_Cameras.push_back(cam);
        }
        ~CameraManager() = default;

        int AddCamera(std::shared_ptr<Camera> cam) { m_Cameras.push_back(cam); return m_Cameras.size(); }
        
        bool Update(float deltaTime) {
            // Handle rotation
            auto [mouseX, mouseY] = Input::GetMousePosition();
            glm::vec2 delta = 0.002f * glm::vec2(mouseX - m_LastMouseX, mouseY - m_LastMouseY);

            std::swap(mouseX, m_LastMouseX);
            std::swap(mouseY, m_LastMouseY);

            if (!Input::IsMouseButtonPressed(AHO_MOUSE_BUTTON_RIGHT)) {
                Input::UnlockCursor();
                return false;
            }

            Input::LockCursor();

            auto cam = GetMainEditorCamera();
            if (delta.x != 0 || delta.y != 0) {
                float pitchDelta = delta.y * cam->GetRotationSpeed();
                float yawDelta = delta.x * cam->GetRotationSpeed();

                glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, cam->GetRight()), glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
                cam->SetForwardRotation(q);
            }

            // Handle WASD movement
            glm::vec3 movement(0.0f, 0.0f, 0.0f);
            if (Input::IsKeyPressed(AHO_KEY_W)) {
                cam->MoveForward(deltaTime);
                //movement.z -= 1.0f;
            }
            else if (Input::IsKeyPressed(AHO_KEY_S)) {
                cam->MoveBackward(deltaTime);
                //movement.z += 1.0f;
            }
            else if (Input::IsKeyPressed(AHO_KEY_A)) {
                cam->MoveLeft(deltaTime);
                //movement.x -= 1.0f;
            }
            else if (Input::IsKeyPressed(AHO_KEY_D)) {
                cam->MoveRight(deltaTime);
                //movement.x += 1.0f;
            }
            //GetMainEditorCamera()->Update(deltaTime, movement);
            return true;
        }

        std::shared_ptr<Camera> GetMainEditorCamera() { AHO_CORE_ASSERT(!m_Cameras.empty()); return m_Cameras[0]; }

    private:
        float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
        std::vector<std::shared_ptr<Camera>> m_Cameras;
    };
} // namespace Aho
