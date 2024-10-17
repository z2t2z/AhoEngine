#pragma once

#include "Runtime/Function/Camera/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Aho {
    class EditorCamera : public Camera {
    public:
        EditorCamera() = default;
        EditorCamera(float fov, float aspectRatio, float nearPlane, float farPlane);
        ~EditorCamera() override = default;

        glm::vec3 GetPosition()       const override { return m_Position; }
        const glm::mat4& GetProjection()     const override { return m_ProjectionMatrix; }
        const glm::mat4& GetProjectionInv() override { m_ProjectionMatrixInv = glm::inverse(m_ProjectionMatrix); return m_ProjectionMatrixInv; }
        const glm::mat4& GetView()           const override { return m_ViewMatrix; }
        const glm::mat4& GetViewInv() override { m_ViewMatrixInv = glm::inverse(m_ViewMatrix); return m_ViewMatrixInv; }
        glm::vec3 GetFront()          const override { return m_Front; }
        glm::vec3 GetRight()          const override { return m_Right; }
        virtual float GetFOV() const override { return m_Fov; }
        virtual float GetAspectRatio() const override { return m_AspectRatio; }
        
        const float GetMoveSpeed()                  const override { return m_Speed; }
        const float GetRotationSpeed()              const override { return m_RotateSpeed; }

        void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) override;
        void SetProjection(const glm::mat4& projection) override { m_ProjectionMatrix = projection; }
        void SetForwardRotation(const glm::quat& q) override { m_Front = glm::rotate(q, m_Front); RecalculateViewMatrix(); }

        void Update(float deltaTime, glm::vec3& movement) override;

        void MoveForward(float speed)   override { m_Position += speed * m_Front; RecalculateViewMatrix(); }
        void MoveBackward(float speed)  override { m_Position -= speed * m_Front; RecalculateViewMatrix(); }
        void MoveLeft(float speed)      override { m_Position -= speed * m_Right; RecalculateViewMatrix(); }
        void MoveRight(float speed)     override { m_Position += speed * m_Right; RecalculateViewMatrix(); }

    private:
        void RecalculateViewMatrix();
        void RecalculateProjectionMatrix();

        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewMatrixInv;
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ProjectionMatrixInv;

        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

        float m_Speed = 1.0f;
        float m_RotateSpeed = 1.0f;
        float m_Fov = 45.0f;
        float m_Yaw;
        float m_Pitch;
        float m_AspectRatio;
        float m_NearPlane = 0.1f;
        float m_FarPlane = 1000.0f;
    };

} // namespace Aho