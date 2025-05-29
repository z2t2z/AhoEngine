#include "Ahopch.h"
#include "CameraManager.h"
#include "Runtime/Core/Math/Math.h"

namespace Aho {
	bool CameraManager::Update(float deltaTime, bool firstClick) {
        auto [mouseX, mouseY] = Input::GetMousePosition();
        // Do nothing if first click because last mouse position is trash data
        if (firstClick) {
            m_LastRight = m_LastForward = 0.0f;
            std::swap(mouseX, m_LastMouseX);
            std::swap(mouseY, m_LastMouseY);
            return false;
        }
        Input::SetCursorPos(m_LastMouseX, m_LastMouseY);

        float sensScale = 1.0f / 2000.0f;
        
        // Handle rotation
        glm::vec2 delta = m_Sensitivity * sensScale * glm::vec2(mouseX - m_LastMouseX, mouseY - m_LastMouseY);
        auto cam = GetMainEditorCamera();
        if (delta.x != 0.0 || delta.y != 0.0) {
            float pitchDelta = delta.y * cam->GetRotationSpeed();
            float yawDelta = delta.x * cam->GetRotationSpeed();

            if (std::max(abs(pitchDelta), abs(yawDelta)) > 0.22f) {
                m_Dirty = false;
                return false;
            }

            glm::quat q = glm::normalize(glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))
                * glm::angleAxis(-pitchDelta, cam->GetRight()));

            cam->SetForwardRotation(q);
        }

        float forwardInput = 0.0f;
        if (Input::IsKeyPressed(AHO_KEY_W)) forwardInput += 1.0f;
        if (Input::IsKeyPressed(AHO_KEY_S)) forwardInput -= 1.0f;

        float rightInput = 0.0f;
        if (Input::IsKeyPressed(AHO_KEY_D)) rightInput += 1.0f;
        if (Input::IsKeyPressed(AHO_KEY_A)) rightInput -= 1.0f;

        forwardInput *= m_Speed;
        rightInput *= m_Speed;

        ApplyMomentum(m_LastRight, rightInput, deltaTime);
        ApplyMomentum(m_LastForward, forwardInput, deltaTime);

        cam->MoveForward(m_LastForward * deltaTime); 
        cam->MoveRight(m_LastRight * deltaTime);

        m_Dirty = true;
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