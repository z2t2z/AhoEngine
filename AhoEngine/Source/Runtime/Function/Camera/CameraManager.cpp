#include "Ahopch.h"
#include "CameraManager.h"

namespace Aho {
	bool CameraManager::Update(float deltaTime, bool firstClick) {
        // Handle rotation
        auto [mouseX, mouseY] = Input::GetMousePosition();
        if (firstClick) {
            m_YawLerpT = 0.0f;
            m_CurrSpeed = 0.0f;
            std::swap(mouseX, m_LastMouseX);
            std::swap(mouseY, m_LastMouseY);
            return false;
        }
        Input::SetCursorPos(m_LastMouseX, m_LastMouseY);

        float sensScale = 1.0f / 2000.0f;
        glm::vec2 delta = m_Sensitivity * sensScale * glm::vec2(mouseX - m_LastMouseX, mouseY - m_LastMouseY);

        auto cam = GetMainEditorCamera();
        if (delta.x != 0 || delta.y != 0) {
            float pitchDelta = delta.y * cam->GetRotationSpeed();
            float yawDelta = delta.x * cam->GetRotationSpeed();

            if (std::max(abs(pitchDelta), abs(yawDelta)) > 0.22f) {
                return false;
            }

            glm::quat q = glm::normalize(glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))
                * glm::angleAxis(-pitchDelta, cam->GetRight()));

            cam->SetForwardRotation(q);
        }
        m_CurrSpeed += deltaTime * m_Acceleration;
        if (m_CurrSpeed > m_Speed) {
            m_CurrSpeed = m_Speed;
        }

        if (Input::IsKeyPressed(AHO_KEY_W)) {
            cam->MoveForward(deltaTime * m_CurrSpeed);
        }
        if (Input::IsKeyPressed(AHO_KEY_S)) {
            cam->MoveBackward(deltaTime * m_CurrSpeed);
        }
        if (Input::IsKeyPressed(AHO_KEY_A)) {
            cam->MoveLeft(deltaTime * m_CurrSpeed);
        }
        if (Input::IsKeyPressed(AHO_KEY_D)) {
            cam->MoveRight(deltaTime * m_CurrSpeed);
        }

        return true;
	}
}