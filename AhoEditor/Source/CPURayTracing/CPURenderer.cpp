#include "CPURenderer.h"

namespace Aho {
	namespace Utils {
		static uint32_t ConvertToRGBA(const glm::vec4& color) {
			uint8_t r = (uint8_t)(color.r * 255.0f);
			uint8_t g = (uint8_t)(color.g * 255.0f);
			uint8_t b = (uint8_t)(color.b * 255.0f);
			uint8_t a = (uint8_t)(color.a * 255.0f);

			uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
			return result;
		}
	}
	
	CPURenderer::CPURenderer() {
		m_TexSpec.Width = 1280;
		m_TexSpec.Height = 960;
		m_TexSpec.GenerateMips = false;
		m_FinalImage = Texture2D::Create(m_TexSpec);
	}

	void CPURenderer::OnResize(uint32_t width, uint32_t height) {
		if (m_FinalImage and m_FinalImage->GetWidth() == width and m_FinalImage->GetHeight() == height) {
			return;
		}

		m_TexSpec.Width = width;
		m_TexSpec.Height = height;
		m_TexSpec.GenerateMips = false;
		m_FinalImage = Texture2D::Create(m_TexSpec);

		delete[] m_ImageData;
		m_ImageData = new uint32_t[width * height];

		delete[] m_AccumulationData;
		m_AccumulationData = new glm::vec4[width * height];
	}

	void CPURenderer::Render(const CPUScene& scene) {
		uint32_t width = m_FinalImage->GetWidth();
		uint32_t height = m_FinalImage->GetHeight();
		if (m_FrameIndex == 1) {
			memset(m_AccumulationData, 0, width * height * sizeof(glm::vec4));
		}

		for (uint32_t x = 0; x < width; x++) {
			for (uint32_t y = 0; y < height; y++) {
				glm::vec4 color = PerPixelShading(x, y);
				m_AccumulationData[x + y * width] += color;

				glm::vec4 accumulatedColor = m_AccumulationData[x + y * width];
				accumulatedColor /= (float)m_FrameIndex;

				accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				m_ImageData[x + y * width] = Utils::ConvertToRGBA(accumulatedColor);
			}
		}
	}

	glm::vec4 CPURenderer::PerPixelShading(uint32_t x, uint32_t y) {
		//auto cam = m_CameraManager.GetMainEditorCamera();
		//Ray ray;
		//ray.Origin = cam->GetPosition();
		//ray.Direction = cam->GetFront(); // ??

		//glm::vec3 light(0.0f);
		//glm::vec3 contribution(1.0f);

		//int MAX_BOUNCE = 5;
		//for (int i = 0; i < MAX_BOUNCE; i++) {
		//	CPURenderer::HitInfo info = TraceSingleRay(ray);
		//	if (info.HitDistance < 0.0f) {
		//		glm::vec3 SkyColor = glm::vec3(0.6f, 0.7f, 0.9f);
		//		break;
		//	}


		//}
		return glm::vec4(1.0f);
	}

	CPURenderer::HitInfo CPURenderer::TraceSingleRay(const Ray& ray) {
		return HitInfo();
	}


}

