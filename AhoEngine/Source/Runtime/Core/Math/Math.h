#pragma once

#include <optional>

#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aho {
	struct Ray;
	class BBox;
	struct PrimitiveDesc;
	struct IntersectResult;

	struct alignas(16) Vertex {
		glm::vec3 position;		float u;
		glm::vec3 normal;		float v;
		glm::vec3 tangent;		float _padding;
		//glm::vec2 uv;			float _padding4; float _padding5;

		//Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& uv)
		//	: position(pos), normal(normal), tangent(tangent), uv(uv) { }
		Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec2& uv)
			: position(pos), normal(normal), tangent(tangent), u(uv.x), v(uv.y) {
		}
		Vertex() = default;
	};

	static glm::vec3 QuaternionToEuler(const glm::quat& q) {
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

	static glm::quat EulerToQuaternion(float roll, float pitch, float yaw) {
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

	static glm::mat4 ComposeTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) {
		glm::quat orientation = EulerToQuaternion(rotation.x, rotation.y, rotation.z);
		return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(orientation) * glm::scale(glm::mat4(1.0f), scale);
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

	bool Intersect(const Ray& ray, const BBox& aabb);

	std::optional<IntersectResult> Intersect(const Ray& ray, const PrimitiveDesc* primitive);
	
	Ray GetRayFromScreenSpace(const glm::vec2& coords, const glm::vec2& resolution, const glm::vec3& camPos, const glm::mat4& projInv, const glm::mat4& viewInv);

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