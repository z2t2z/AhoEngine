#pragma once

#define GLM_FORCE_CTOR_INIT
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Aho {
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


}