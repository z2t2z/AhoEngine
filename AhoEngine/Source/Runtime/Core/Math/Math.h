#pragma once

#include <optional>

#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <random>

namespace Aho {
	namespace Utils {
		inline std::mt19937 rng(std::random_device{}());
		inline std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
		// Generate a random vec3 ranging from [-1.0, 1.0]
		inline glm::vec3 GenerateRandomVec3() {
			return glm::vec3(dist(rng), dist(rng), dist(rng));
		}
	}

	struct Ray;
	class BBox;
	struct PrimitiveDesc;
	struct IntersectResult;

	inline glm::vec3 QuaternionToEuler(const glm::quat& q) {
		glm::vec3 euler;

		// Roll (x-axis rotation)
		float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
		float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
		euler.x = std::atan2(sinr_cosp, cosr_cosp);

		// Pitch (y-axis rotation)
		float sinp = 2 * (q.w * q.y - q.z * q.x);
		if (std::abs(sinp) >= 1)
			euler.y = std::copysign(glm::half_pi<float>(), sinp);  // Use 90 degrees if out of range
		else
			euler.y = std::asin(sinp);

		// Yaw (z-axis rotation)
		float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
		float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
		euler.z = std::atan2(siny_cosp, cosy_cosp);

		return euler;
	}

	inline glm::quat EulerToQuaternion(float roll, float pitch, float yaw) {
		roll = glm::radians(roll);
		pitch = glm::radians(pitch);
		yaw = glm::radians(yaw);

		float cy = cos(yaw * 0.5f);
		float sy = sin(yaw * 0.5f);
		float cp = cos(pitch * 0.5f);
		float sp = sin(pitch * 0.5f);
		float cr = cos(roll * 0.5f);
		float sr = sin(roll * 0.5f);

		glm::quat q;
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;
		return q;
	}

	inline static glm::mat4 ComposeTransform(const glm::vec3& Translation, const glm::vec3& Rotation, const glm::vec3& Scale) {
		glm::quat Orientation = EulerToQuaternion(Rotation.x, Rotation.y, Rotation.z);
		return glm::translate(glm::mat4(1.0f), Translation) * glm::toMat4(Orientation) * glm::scale(glm::mat4(1.0f), Scale);
	}
	inline void DecomposeTransform(const glm::mat4& transform, glm::vec3& Translation, glm::vec3& Rotation, glm::vec3& Scale, glm::quat& Orientation) {
		glm::vec3 Skew;
		glm::vec4 Perspective;
		glm::decompose(transform, Scale, Orientation, Translation, Skew, Perspective);
		Rotation = glm::degrees(QuaternionToEuler(Orientation));
	}
	inline void DecomposeTransform(const glm::mat4& transform, glm::vec3& Translation, glm::vec3& Rotation, glm::vec3& Scale) {
		glm::vec3 Skew;
		glm::vec4 Perspective;
		glm::quat Orientation;
		glm::decompose(transform, Scale, Orientation, Translation, Skew, Perspective);
	}
	inline static float SimpleLerp(float a, float b, float t) { return a + (b - a) * t; }

	inline static void Clamp(float& v, float lb, float ub) {
		if (v < lb) {
			v = lb;
		}
		if (v > ub) {
			v = ub;
		}
	}

	extern bool Intersect(const Ray& ray, const BBox& aabb);

	extern std::optional<IntersectResult> Intersect(const Ray& ray, const PrimitiveDesc* primitive);
	
	extern Ray GetRayFromScreenSpace(const glm::vec2& coords, const glm::vec2& resolution, const glm::vec3& camPos, const glm::mat4& projInv, const glm::mat4& viewInv);

	inline static glm::vec3 Min(const glm::vec3& a, const glm::vec3& b) {
		return glm::vec3(
			std::min(a.x, b.x),
			std::min(a.y, b.y),
			std::min(a.z, b.z)
		);
	}

	inline static glm::vec3 Max(const glm::vec3& a, const glm::vec3& b) {
		return glm::vec3(
			std::max(a.x, b.x),
			std::max(a.y, b.y),
			std::max(a.z, b.z)
		);
	}
}