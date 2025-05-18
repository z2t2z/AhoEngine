#pragma once

#include <glad/glad.h>

namespace Aho {
	
	// UBO: uniform buffer object in opengl

	class UBOBase {
	public:
		virtual ~UBOBase() = default;
	};

    template <typename T>
    class UBO : public UBOBase {
    public:
        UBO(uint32_t bindingPoint) : m_BindingPoint(bindingPoint) {
            glGenBuffers(1, &m_BufferID);
            glBindBuffer(GL_UNIFORM_BUFFER, m_BufferID);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(T), nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_BufferID);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        ~UBO() {
            glDeleteBuffers(1, &m_BufferID);
        }

        void SetData(const T& data) {
            glBindBuffer(GL_UNIFORM_BUFFER, m_BufferID);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

		//void GetSubData(T& data, size_t offset) {
		//	glBindBuffer(GL_UNIFORM_BUFFER, m_BufferID);
		//	glGetBufferSubData(GL_UNIFORM_BUFFER, offset * sizeof(T), data.size() * sizeof(T), data.data());

		//}

        uint32_t GetBindingPoint() const { return m_BindingPoint; }

    private:
        uint32_t m_BufferID;
        uint32_t m_BindingPoint;
    };

	// Works totally fine without alignment
	struct alignas(16) CameraUBO {
		CameraUBO() = default;
		glm::mat4 u_View{ 0.0f };
		glm::mat4 u_ViewInv{ 0.0f };
		glm::mat4 u_Projection{ 0.0f };
		glm::mat4 u_ProjectionInv{ 0.0f };
		glm::mat4 u_ProjView{ 0.0f };
		glm::vec4 u_ViewPosition{ 0.0f };
	};

	constexpr int MAX_LIGHT_CNT = 5;
	struct alignas(16) PointLight {
		glm::vec4 position; // world space, radius in w
		glm::vec4 color;    // RGB, intensity in w
		PointLight() = default;
		PointLight(const glm::vec4& pos, const glm::vec4& col) : position(pos), color(col) {}
	};
	struct alignas(16) DirectionalLight {
		glm::vec3 direction; // world space, radius in w
		float _padding;
		glm::vec3 color{ 1 };    // RGB, intensity in w
		float intensity{ 0.0 };
		glm::mat4 lightProjView{ 1 };
		DirectionalLight() = default;
		DirectionalLight(const glm::mat4& mat, const glm::vec3& dir, const glm::vec3& col = glm::vec3(1.0), float intensity = 1.0) 
			: lightProjView(mat), direction(dir), color(col), intensity(intensity) {}
	};
	struct alignas(16) GPU_AreaLight {
		glm::vec4 position{ 0.0f };
		glm::vec4 normal{ 0.0f, 1.0f, 0.0f, 0.0f};
		glm::mat4 transform{ 1.0f };
		glm::vec4 color{ 1.0f };
		glm::vec4 params{ 0.0f };
		GPU_AreaLight() = default;
		GPU_AreaLight(const glm::mat4& transform, const glm::vec3& color, float intensity, float width, float height)
			: transform(transform), color(glm::vec4(color, intensity)), params(glm::vec4(0.0f, width, height, 0.0f)) {
			position = glm::vec4(glm::vec3(transform[3]), 1.0f);
			glm::mat3 normalMatrix3x3 = glm::transpose(glm::inverse(glm::mat3(transform)));
			normal = glm::normalize(glm::vec4(normalMatrix3x3 * glm::vec3(0.0f, 1.0f, 0.0f), 0.0f));
		}
	};

	struct alignas(16) LightUBO {
		PointLight u_PointLight[MAX_LIGHT_CNT];
		DirectionalLight u_DirLight[MAX_LIGHT_CNT];
		GPU_AreaLight u_AreaLight[MAX_LIGHT_CNT];
		glm::uvec4 u_LightCount{ 0u }; // Point, directional, spot, area
		LightUBO() = default;
	};

	constexpr int SAMPLES_CNT = 64;
	// For ssao pass
	struct alignas(16) RandomKernelUBO {
		glm::vec4 u_Samples[SAMPLES_CNT];
		glm::vec4 u_Info{ 0 }; // width, height, radius, bias
		RandomKernelUBO() {
			for (int i = 0; i < SAMPLES_CNT; i++) {
				glm::vec3 sample = Utils::GenerateRandomVec3(); // [-1, 1]
				sample = glm::normalize(sample);
				sample *= Utils::GenerateRandomVec3().x * 0.5 + 0.5f;
				float scale = float(i) / float(SAMPLES_CNT);
				// scale samples s.t. they're more aligned to center of kernel
				scale = 0.1f + 1.0f * (scale * scale);
				sample *= scale;
				u_Samples[i] = glm::vec4(sample, 0.0f);
			}
			u_Info = { 1280.f, 720.0f, 0.5f, 0.025f };
		}
	};

	// For animation, read by vertices
	constexpr int MAX_BONES_CNT = 200;
	struct alignas(16) AnimationUBO {
		glm::mat4 u_BoneMatrices[MAX_BONES_CNT];
		AnimationUBO() {
			for (int i = 0; i < MAX_BONES_CNT; i++) {
				u_BoneMatrices[i] = glm::mat4(1.0f);
			}
		}
	};

	// For visual debugging
	struct alignas(16) SkeletonUBO {
		glm::mat4 u_BoneMatrices[MAX_BONES_CNT];
		glm::ivec4 u_BoneEntityID[MAX_BONES_CNT];
		SkeletonUBO() {
			for (int i = 0; i < MAX_BONES_CNT; i++) {
				u_BoneMatrices[i] = glm::mat4(1.0f);
				u_BoneEntityID[i] = glm::ivec4(10000000);
			}
		}
	};
}
