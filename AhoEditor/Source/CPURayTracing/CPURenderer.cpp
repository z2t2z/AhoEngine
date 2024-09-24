#include "CPURenderer.h"

#include <cstdlib>
#include <ctime>

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

		glm::vec3 GenerateRandomVec3() {
			float x = static_cast<float>(rand()) / RAND_MAX - 0.5f; 
			float y = static_cast<float>(rand()) / RAND_MAX - 0.5f; 
			float z = static_cast<float>(rand()) / RAND_MAX - 0.5f;
			return glm::vec3(x, y, z);
		}
	}
	
	CPURenderer::CPURenderer() {
		m_TexSpec.Width = 128;
		m_TexSpec.Height = 96;
		m_TexSpec.GenerateMips = false;
		m_FinalImage = Texture2D::Create(m_TexSpec);
	}

	void CPURenderer::OnResize(uint32_t width, uint32_t height) {
		if (m_FinalImage and m_FinalImage->GetWidth() == width and m_FinalImage->GetHeight() == height) {
			return;
		}
		//return;
		m_TexSpec.Width = width;
		m_TexSpec.Height = height;
		m_TexSpec.GenerateMips = false;
		m_FinalImage = Texture2D::Create(m_TexSpec);

		delete[] m_ImageData;
		m_ImageData = new uint32_t[width * height];

		delete[] m_AccumulationData;
		m_AccumulationData = new glm::vec4[width * height];
	}

	void CPURenderer::Render(const CPUScene& scene, const CameraManager& cameraManager) {
		//AHO_TRACE("{}, {}", m_Memo.size(), m_FinalImage->GetWidth() * m_FinalImage->GetHeight());
		//AHO_ASSERT(m_FinalImage->GetWidth() * m_FinalImage->GetHeight() == m_Memo.size());
		m_ActiveScene = &scene;
		m_CameraManager = cameraManager;

		uint32_t width = m_FinalImage->GetWidth();
		uint32_t height = m_FinalImage->GetHeight();

		//AHO_TRACE("{}, {}", width, height);

		//PerPixelShading(width / 2, height / 2);
		//return;

		if (m_FrameIndex == 1) {
			memset(m_AccumulationData, 0, width * height * sizeof(glm::vec4));
		}

		for (uint32_t x = 0; x < width; x += 1) {
			for (uint32_t y = 0; y < height; y += 1) {
				glm::vec4 color = PerPixelShading(x, y);
				m_AccumulationData[x + y * width] += color;

				glm::vec4 accumulatedColor = m_AccumulationData[x + y * width];
				accumulatedColor /= (float)m_FrameIndex;

				accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
				//m_ImageData[x + y * width] = Utils::ConvertToRGBA(glm::vec4(Utils::GenerateRandomVec3(), 1.0f));
				m_ImageData[x + y * width] = Utils::ConvertToRGBA(accumulatedColor);
			}
		}

		m_FinalImage->SetData(m_ImageData, width * height * 4);

		if (m_Settings.Accumulate)
			m_FrameIndex++;
		else
			m_FrameIndex = 1;
	}

	glm::vec4 CPURenderer::PerPixelShading(uint32_t x, uint32_t y) {
		auto cam = m_CameraManager.GetMainEditorCamera();
		Ray ray = RayCasting(x, y);

		//AHO_INFO("{}, {}", x, y);
		//AHO_INFO("{},{},{}", ray.Origin.x, ray.Origin.y, ray.Origin.z);
		//AHO_INFO("{},{},{}", ray.Direction.x, ray.Direction.y, ray.Direction.z);
		//return glm::vec4(1.0f);

		glm::vec3 color(0.0f);
		glm::vec3 contribution(1.0f);

		float multiplier = 1.0f;

		int MAX_BOUNCE = 4;
		for (int i = 0; i < MAX_BOUNCE; i++) {
			CPURenderer::HitInfo hitInfo = TraceSingleRay(ray);
			if (hitInfo.HitDistance < 0.0f) {
				glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
				color += skyColor * multiplier;
				break;
			}
			
			glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

			float lightIntensity = glm::max(glm::dot(hitInfo.WorldNormal, -lightDir), 0.0f); // == cos(angle)

			const Sphere& sphere = m_ActiveScene->Spheres[hitInfo.ObjectIndex];
			const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

			contribution *= material.Albedo;

			glm::vec3 sphereColor = material.Albedo;
			sphereColor *= lightIntensity;
			color += sphereColor * multiplier;

			ray.Origin = hitInfo.WorldPosition + hitInfo.WorldNormal * 0.0001f;
			ray.Direction = glm::reflect(ray.Direction, hitInfo.WorldNormal + material.Roughness * Utils::GenerateRandomVec3());
		}

		return glm::vec4(color, 1.0f);
	}

	Ray CPURenderer::RayCasting(uint32_t x, uint32_t y) {
		if (m_Memo.contains(x * 100000 + y)) {
			return m_Memo[x * 100000 + y];
		}

		auto cam = m_CameraManager.GetMainEditorCamera();
		Ray ray;
		ray.Origin = cam->GetPosition();

		// Image space to screen space
		uint32_t width = m_FinalImage->GetWidth();
		uint32_t height = m_FinalImage->GetHeight();
		float ssx = (float)x / width, ssy = (float)y / height;

		// Screen space to NDC
		glm::vec2 ndc = glm::vec2(ssx * 2.0f - 1.0f, ssy * 2.0f - 1.0f);

		// NDC to view space
		glm::vec4 vs = cam->GetProjectionInv() * glm::vec4(ndc, 1.0f, 1.0f);
		vs.z = -1.0f;
		vs.w = 0.0f;

		// View space to world space
		glm::vec3 ws = cam->GetViewInv() * vs;
		ray.Direction = glm::normalize(ws);

		m_Memo[x * 100000 + y] = ray;
		return ray;
	}

	CPURenderer::HitInfo CPURenderer::TraceSingleRay(const Ray& ray) {
		int closestSphere = -1;
		float hitDistance = std::numeric_limits<float>::max();
		for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++) {
			const Sphere& sphere = m_ActiveScene->Spheres[i];
			glm::vec3 origin = ray.Origin - sphere.Position;

			float a = glm::dot(ray.Direction, ray.Direction);
			float b = 2.0f * glm::dot(origin, ray.Direction);
			float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

			// Quadratic forumula discriminant:
			// b^2 - 4ac

			float discriminant = b * b - 4.0f * a * c;
			if (discriminant < 0.0f)
				continue;

			// Quadratic formula:
			// (-b +- sqrt(discriminant)) / 2a

			// float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a); // Second hit distance (currently unused)
			float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
			if (closestT > 0.0f && closestT < hitDistance) {
				hitDistance = closestT;
				closestSphere = (int)i;
			}
		}
		if (closestSphere < 0) {
			return Miss(ray);
		}

		return ClosestHit(ray, hitDistance, closestSphere);
	}

	CPURenderer::HitInfo CPURenderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex) {
		CPURenderer::HitInfo hitInfo;
		hitInfo.HitDistance = hitDistance;
		hitInfo.ObjectIndex = objectIndex;

		const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

		glm::vec3 origin = ray.Origin - closestSphere.Position;
		hitInfo.WorldPosition = origin + ray.Direction * hitDistance;
		hitInfo.WorldNormal = glm::normalize(hitInfo.WorldPosition);

		hitInfo.WorldPosition += closestSphere.Position;

		return hitInfo;
	}

	CPURenderer::HitInfo CPURenderer::Miss(const Ray& ray) {
		CPURenderer::HitInfo hitInfo;
		hitInfo.HitDistance = -1.0f;
		return hitInfo;
	}
}

