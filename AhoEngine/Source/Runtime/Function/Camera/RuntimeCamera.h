#pragma once

#include "Camera.h"

namespace Aho {
    //class RuntimeCamera : public Camera {
    //public:
    //    RuntimeCamera() = default;
    //    RuntimeCamera(float fov, float aspectRatio, float nearPlane, float farPlane) {}

    //    inline const glm::vec3 GetPosition()       const override { return m_Position; }
    //    inline const glm::mat4 GetView()           const override { return m_ViewMatrix; }
    //    inline const glm::mat4 GetProjection()     const override { return m_ProjectionMatrix; }

    //    inline void SetProjection(const glm::mat4& projection) override { m_ProjectionMatrix = projection; }
    //    void SetProjection(float fov, float aspectRatio, float nearPlane, float farPlane) override {}
    //    void Update(float deltaTime, glm::vec3& movement) override {}

    //private:
    //    void RecalculateViewMatrix() {}
    //    void RecalculateProjectionMatrix() {}

    //    glm::mat4 m_ViewMatrix;
    //    glm::mat4 m_ProjectionMatrix;

    //    glm::vec3 m_Position = glm::vec3(0.0f);
    //    glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
    //    glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);

    //    //float m_Yaw;
    //    //float m_Pitch;
    //    float m_Fov;
    //    float m_AspectRatio;
    //    float m_NearPlane;
    //    float m_FarPlane;
    //};

} // namespace Aho