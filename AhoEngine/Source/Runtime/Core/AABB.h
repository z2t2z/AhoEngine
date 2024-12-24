#pragma once

#include "Math/Math.h"
#include "Ray.h"
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
		void Merge(const AABB& other) {
			m_Pmin = glm::min(m_Pmin, other.m_Pmin);
			m_Pmax = glm::max(m_Pmax, other.m_Pmax);
		}

		void Merge(const glm::vec3& p) {
			m_Pmin = glm::min(m_Pmin, p);
			m_Pmax = glm::max(m_Pmax, p);
		}

		static AABB Merge(const AABB& lhs, const AABB& rhs);
		static AABB Merge(const AABB& lhs, const glm::vec3& p);

		bool Overlaps(const AABB& other) {
			return false;
		}

		glm::vec3 Offset(const glm::vec3& p) const {
			glm::vec3 o = p - m_Pmin;
			if (m_Pmax.x > m_Pmin.x) o.x /= m_Pmax.x - m_Pmin.x;
			if (m_Pmax.y > m_Pmin.y) o.y /= m_Pmax.y - m_Pmin.y;
			if (m_Pmax.z > m_Pmin.z) o.z /= m_Pmax.z - m_Pmin.z;
			return o;
		}

		bool Intersect(const Ray& ray) const;

		glm::vec3 GetCentroid() const { return glm::vec3(0.5) * m_Pmin + glm::vec3(0.5) * m_Pmax; }

		// Diagonal
		glm::vec3 GetAbsDiff() const {
			return m_Pmax - m_Pmin;
		}

		float GetSurfaceArea() const {
			glm::vec3 d = GetAbsDiff();
			return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
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