#pragma once

#include <glm/glm.hpp>

namespace Aho {
	enum class LightType { Directional, Point, Spot, Area };

    // Infinite area light(IBL) does not use this interface
    class Light {
    public:
        Light(LightType type, const glm::vec3& color, float intensity = 1.0f, bool castShadow = true)
            : m_Color(color), m_Type(type), m_Intensity(intensity), m_CastShadow(castShadow) {
        }
        virtual ~Light() = default;
        LightType GetType() const { return m_Type; }
        float& GetIntensity() { return m_Intensity; }
        glm::vec3& GetColor() { return m_Color; }
        bool& CastShadow() { return m_CastShadow; }
    protected:
        glm::vec3 m_Color;
        LightType m_Type;
        bool m_CastShadow{ true };
        float m_Intensity;
    };

    enum class Shape {Rectangle, Triangle, Circle};
    class AreaLight : public Light {
    public:
        AreaLight() 
            : Light(LightType::Area, glm::vec3(1.0f), 1.0f, true), m_Width(1.0f), m_Height(1.0f), m_Shape(Shape::Rectangle) {}
        AreaLight(const glm::vec3& color, float intensity = 1.0f, Shape shape = Shape::Rectangle, bool castShadow = true)
            : Light(LightType::Area, color, intensity, castShadow), m_Shape(shape), m_Width(1.0f), m_Height(1.0f) { }
        Shape GetLightShape() const { return m_Shape; }
        float GetHeight() const { return m_Height; }
        float GetWidth() const { return m_Width; }
    private:
        float m_Width, m_Height;
        Shape m_Shape;
    };

    class DirectionalLight : public Light {
    public:
        DirectionalLight()
            : Light(LightType::Directional, glm::vec3(1.0f), 1.0f, true), m_Direction(glm::vec3(0.0f, 1.0f, 0.0f)) {
            CalProjViewMat();
        }
        // Getters
        glm::vec3 GetDirection()    const { return m_Direction; }
		glm::vec3 GetWorldCenter()  const { return m_WorldCenter; }  
		float GetLightDistance()    const { return m_LightDistance; }
		float GetOrthoSize()        const { return m_OrthoSize; }
        float GetNearPlane()        const { return m_Near; }
		float GetFarPlane()         const { return m_Far; } 

        // Setters
        void SetDirection(const glm::vec3& dir)         { m_Direction = dir; }
		void SetWorldCenter(const glm::vec3& center)    { m_WorldCenter = center; }
		void SetLightDistance(float distance)           { m_LightDistance = distance; }
		void SetOrthoSize(float size)                   { m_OrthoSize = size; }
		void SetNearPlane(float nearPlane)              { m_Near = nearPlane; }
		void SetFarPlane(float farPlane)                { m_Far = farPlane; }

        void SetProjView(const glm::mat4& mat) { m_ProjView = mat; }
        glm::mat4 GetProjView() { CalProjViewMat(); return m_ProjView; }
    private:
        void CalProjViewMat();
    private:
        // TODO: Needs reflection
        glm::vec3 m_WorldCenter{ 0 };
		float m_LightDistance{ 20.0f };
		float m_OrthoSize{ 10.0f };
        float m_Near{ 1.0f };
        float m_Far{ 500.0f };
        float m_LightSize{ 10.0f }; // PCSS
        glm::vec3 m_Direction; // In euler angles
        glm::mat4 m_ProjView;
    };
}