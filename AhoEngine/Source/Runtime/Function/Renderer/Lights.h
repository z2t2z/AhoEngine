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
}