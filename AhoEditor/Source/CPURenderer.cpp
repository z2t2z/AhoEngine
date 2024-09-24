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
		//m_FinalImage = Texture2D::Create(m_TexSpec);
	}

	void CPURenderer::OnResize(uint32_t width, uint32_t height) {
		//if (m_FinalImage and m_FinalImage->GetWidth() == width and m_FinalImage->GetHeight() == height) {
		//	return;
		//}

		m_TexSpec.Width = width;
		m_TexSpec.Height = height;
		m_TexSpec.GenerateMips = false;
		//m_FinalImage = std::make_shared<Texture2D>(m_TexSpec);

		delete[] m_ImageData;
		m_ImageData = new uint32_t[width * height];

		delete[] m_AccumulationData;
		m_AccumulationData = new glm::vec4[width * height];
	}


}

