#pragma once

#include "Runtime/Core/BVH.h"
#include <glm/glm.hpp>
#include <vector>

namespace Aho {
	namespace CPU {
		struct Material {
			glm::vec3 Albedo{ 1.0f };
			float Roughness = 1.0f;
			float Metallic = 0.0f;
			glm::vec3 EmissionColor{ 0.0f };
			float EmissionPower = 0.0f;

			glm::vec3 GetEmission() const { return EmissionColor * EmissionPower; }
		};
	}
	//struct Ray {
	//	glm::vec3 Origin;
	//	glm::vec3 Direction;
	//};

	struct Sphere {
		glm::vec3 Position{ 0.0f };
		float Radius = 0.5f;

		int MaterialIndex = 0;
	};

	struct CPUScene {
		std::vector<Sphere> Spheres;
		std::vector<CPU::Material> Materials;
		CameraManager* m_CameraManager;

		CPUScene() : m_CameraManager(new CameraManager()) {}
		~CPUScene() {
			delete m_CameraManager; 
		}
	};
} // namespace Aho