#pragma once
#include "IamAho.h"

#include "CPUScene.h"
#include <memory>
#include <unordered_map>

namespace Aho {
	class CPURenderer {
	public:
		struct Settings {
			bool Accumulate = true;
		};
		
	public:
		CPURenderer();

		void OnResize(uint32_t width, uint32_t height);
		void Render(const CPUScene& scene);

		std::shared_ptr<Texture2D> GetFinalImage() const { return m_FinalImage; }

		void ResetFrameIndex() { m_FrameIndex = 1; }
		Settings& GetSettings() { return m_Settings; }
	private:

		struct HitInfo {
			float HitDistance;
			glm::vec3 WorldPosition;
			glm::vec3 WorldNormal;

			int ObjectIndex;
		};

		glm::vec4 PerPixelShading(const CPUScene& scene, uint32_t x, uint32_t y); // RayGen
		Ray RayCasting(const std::shared_ptr<Camera>& cam, uint32_t x, uint32_t y);
		HitInfo TraceSingleRay(const CPUScene& scene, const Ray& ray);
		HitInfo ClosestHit(const CPUScene& scene, const Ray& ray, float hitDistance, int objectIndex);
		HitInfo Miss(const Ray& ray);
	private:
		std::unordered_map<uint32_t, Ray> m_Memo;

		TextureSpecification m_TexSpec;
		std::shared_ptr<Texture2D> m_FinalImage;
		Settings m_Settings;

		std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter; // multi-threading

		uint32_t* m_ImageData = nullptr;
		glm::vec4* m_AccumulationData = nullptr;
		uint32_t m_FrameIndex = 1;
	};

}