#pragma once
#include <glad/glad.h>

namespace Aho {
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
		glm::vec4 u_ViewPosition{ 0.0f };
	};

	constexpr int MAX_LIGHT_CNT = 10;
	struct alignas(16) LightUBO {
		glm::mat4 u_LightPV[MAX_LIGHT_CNT];
		glm::vec4 u_LightPosition[MAX_LIGHT_CNT];
		glm::vec4 u_LightColor[MAX_LIGHT_CNT];
		glm::ivec4 u_Info[MAX_LIGHT_CNT]; // Enabled status; Light type; ...
		LightUBO() {
			for (int i = 0; i < MAX_LIGHT_CNT; i++) {
				u_LightPosition[i] = u_LightColor[i] = glm::vec4(0.0f);
				u_LightPosition[i].w = 1.0f;
				u_Info[i] = glm::ivec4(0);
			}
		}
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
		//std::array<uint32_t, MAX_BONES_CNT> u_BoneEntityID{ 0 };
		//uint32_t u_BoneEntityID[MAX_BONES_CNT];
		glm::ivec4 u_BoneEntityID[MAX_BONES_CNT];
		SkeletonUBO() {
			for (int i = 0; i < MAX_BONES_CNT; i++) {
				u_BoneMatrices[i] = glm::mat4(1.0f);
				u_BoneEntityID[i] = glm::ivec4(10000000);
			}
		}
	};
}
