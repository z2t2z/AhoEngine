#include "Ahopch.h"
#include "PathTracingPipeline.h"

#include <memory>
#include <execution>

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

	PathTracingPipeline::PathTracingPipeline() {
		m_TexSpec.dataFormat = TexDataFormat::RGBA;
		m_TexSpec.internalFormat = TexInterFormat::RGBA32F;
		m_TexSpec.type = TexType::Result;

		m_FinalImage = Texture2D::Create(m_TexSpec);
		OnResize(1280, 721);

		Initialize();
	}

	void PathTracingPipeline::Initialize() {

	}

	void PathTracingPipeline::Execute() {

	}

	void PathTracingPipeline::OnResize(uint32_t width, uint32_t height) {
		if (m_FinalImage and m_FinalImage->GetWidth() == width and m_FinalImage->GetHeight() == height) {
			return;
		}
		// Leakage?
		//m_SSBO.reset(ShaderStorageBuffer::Create(width * height * sizeof(uint32_t)));
		//m_SSBOAccumulate.reset(ShaderStorageBuffer::Create(width * height * sizeof(glm::vec4)));
		m_TexSpec.width = width;
		m_TexSpec.height = height;
		m_TexSpec.dataFormat = TexDataFormat::RGBA;
		m_TexSpec.internalFormat = TexInterFormat::RGBA32F;
		m_FinalImage = Texture2D::Create(m_TexSpec);
		m_Noise = Texture2D::Create(m_TexSpec);
		//Utils::FillRandom(m_Random, width * height * 3);
		//m_Noise->SetData(m_Random.data(), m_Random.size());

		delete[] m_ImageData;
		m_ImageData = new uint32_t[width * height];

		delete[] m_AccumulationData;
		m_AccumulationData = new glm::vec4[width * height];

		m_ImageHorizontalIter.resize(width);
		m_ImageVerticalIter.resize(height);

		std::iota(m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(), 0u);
		std::iota(m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(), 0u);
	}

}