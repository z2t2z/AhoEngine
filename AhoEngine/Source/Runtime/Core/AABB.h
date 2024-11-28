#pragma once

#include "Math/Math.h"
#include <limits>

namespace Aho {
	enum class Axis {
		X = 0,
		Y = 1,
		Z = 2
	};

	class AABB {
	public:
		AABB() : m_Pmin(glm::vec3(std::numeric_limits<float>::max())), m_Pmax(glm::vec3(std::numeric_limits<float>::min())) {}
		AABB(const glm::vec3& p0, const glm::vec3& p1) {
			m_Pmin = glm::min(p0, p1);
			m_Pmax = glm::max(p0, p1);
		}

		// Expand current AABB to include another AABB
		void Expand(const AABB& other) {
			m_Pmin = glm::min(m_Pmin, other.m_Pmin);
			m_Pmax = glm::max(m_Pmax, other.m_Pmax);
		}

		bool Overlaps(const AABB& other) {
			return false;
		}

		glm::vec3 GetCentroid() { return glm::vec3(0.5) * m_Pmin + glm::vec3(0.5) * m_Pmax; }

		glm::vec3 GetAbsDiff() {
			return m_Pmax - m_Pmin;
		}

		Axis GetSplitAxis() {
			glm::vec3 d = GetAbsDiff();
			return d.x > d.y && d.x > d.z ? Axis::X
				: d.y > d.z ? Axis::Y : Axis::Z;
		}

		glm::vec3 GetMin() const { return m_Pmin; }
		glm::vec3 GetMax() const { return m_Pmax; }

	private:
		glm::vec3 m_Pmin;
		glm::vec3 m_Pmax;
	};
}