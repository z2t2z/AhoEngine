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

	PathTracerPipeline::PathTracerPipeline() {
		m_TexSpec.dataFormat = TexDataFormat::RGBA;
		m_TexSpec.internalFormat = TexInterFormat::RGBA32F;
		m_TexSpec.type = TexType::Result;

		m_FinalImage = Texture2D::Create(m_TexSpec);
		OnResize(1280, 721);
	}

	void PathTracerPipeline::Execute() {
		std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
			[&](uint32_t y) {
				std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
					[&](uint32_t x) {
						glm::vec4 color = PerPixelShading(x, y);
						m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

						glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
						accumulatedColor /= (float)m_FrameIndex;

						accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
						m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
					});
			});

		m_FinalImage->SetData(m_ImageData, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * 4);

		m_Accumulate ? m_FrameIndex++ : m_FrameIndex = 1;
	}

	void PathTracerPipeline::OnResize(uint32_t width, uint32_t height) {
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

	glm::vec4 PathTracerPipeline::SampleRadiance(IntersectResult result) {
		//return result.albedo;
		return glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	glm::vec4 PathTracerPipeline::PerPixelShading(uint32_t x, uint32_t y) {
		Ray ray = GetRayFromScreenSpace(glm::vec2(x, y),
			glm::vec2(m_FinalImage->GetWidth(), m_FinalImage->GetHeight()),
			m_Camera->GetPosition(),
			m_Camera->GetProjectionInv(),
			m_Camera->GetViewInv());
		glm::vec3 color(0.0f);
		glm::vec3 contribution(1.0f);

		float multiplier = 1.0f;

		int MAX_BOUNCE = 1;
		for (int i = 0; i < MAX_BOUNCE; i++) {
			auto intersectResult = BVH::GetIntersection(ray, m_Root);

			if (!intersectResult) {
				//glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
				//color += skyColor;
				break;
			}

			AHO_CORE_TRACE("Intersect!");

			glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

			float lightIntensity = glm::max(glm::dot(intersectResult->normal, -lightDir), 0.0f); // == cos(angle)

			//const Sphere& sphere = scene.Spheres[hitInfo.ObjectIndex];
			//const CPU::Material& material = scene.Materials[sphere.MaterialIndex];

			//contribution *= material.Albedo;

			//glm::vec3 sphereColor = material.Albedo;
			//sphereColor *= lightIntensity;
			color += lightIntensity * SampleRadiance(intersectResult.value());
			//color += intersectResult->albedo;

			//ray.origin = hitInfo.WorldPosition + hitInfo.WorldNormal * 0.0001f;
			//ray.Direction = glm::reflect(ray.Direction, hitInfo.WorldNormal + material.Roughness * Utils::GenerateRandomVec3());
			//ray.direction = glm::reflect(ray.direction, hitInfo.WorldNormal/* + material.Roughness * m_Random[y * m_TexSpec.width + x]*/);
		}
		return glm::vec4(color, 1.0f);
	}

}