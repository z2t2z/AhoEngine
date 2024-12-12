#include "Ahopch.h"
#include "CameraManager.h"
#include "Runtime/Core/Math/Math.h"

namespace Aho {
	bool CameraManager::Update(float deltaTime, bool firstClick) {
        // Handle rotation
        auto [mouseX, mouseY] = Input::GetMousePosition();
        if (firstClick) {
            std::swap(mouseX, m_LastMouseX);
            std::swap(mouseY, m_LastMouseY);
            m_LastForward = 0.0f;
            m_LastRight = 0.0f;
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

        float forward = m_Speed *
            ((Input::IsKeyPressed(AHO_KEY_W) ? deltaTime : 0.0f) +
                (Input::IsKeyPressed(AHO_KEY_S) ? -deltaTime : 0.0f));

        float right = m_Speed *
            ((Input::IsKeyPressed(AHO_KEY_D) ? deltaTime : 0.0f) +
                (Input::IsKeyPressed(AHO_KEY_A) ? -deltaTime : 0.0f));

        ApplyMomentum(m_LastRight, right, deltaTime);
        ApplyMomentum(m_LastForward, forward, deltaTime);

        cam->MoveForward(forward);
        cam->MoveRight(right);

        return true;
	}

    void CameraManager::ApplyMomentum(float& oldValue, float& newValue, float deltaTime) {
        float blendedValue;
        if (std::abs(newValue) > std::abs(oldValue)) {
            blendedValue = SimpleLerp(newValue, oldValue, std::pow(0.6f, deltaTime * 60.0f));
        } 
        else {
            blendedValue = SimpleLerp(newValue, oldValue, std::pow(0.8f, deltaTime * 60.0f));
        }
        oldValue = blendedValue;
        newValue = blendedValue;
    }
}