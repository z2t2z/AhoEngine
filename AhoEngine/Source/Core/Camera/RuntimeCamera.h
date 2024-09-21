#pragma once

#include "Camera.h"

namespace Aho {
    class RuntimeCamera : public Camera {
    public:
        RuntimeCamera() = default;
        RuntimeCamera(float fov, float aspectRatio, float nearPlane, float farPlane);

        const glm::mat4& GetView() const override;
        const glm::mat4& GetProjection() const override;
        void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) override;
        void SetProjection(const glm::mat4& projection) override;
        void Update(float deltaTime) override;

    private:
        void RecalculateViewMatrix();
        void RecalculateProjectionMatrix();

        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionMatrix;

        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_up;

        float m_yaw;
        float m_pitch;
        float m_fov;
        float m_aspectRatio;
        float m_nearPlane;
        float m_farPlane;
    };

} // namespace Aho