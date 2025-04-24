#pragma once

#include "Math/Math.h"
#include <limits>

namespace Aho {
	enum class Axis {
		X = 0,
		Y = 1,
		Z = 2
	};
	struct Ray;

	static const float inf = 1E25;
	class alignas(16) BBox {
	public:
		bool operator==(const BBox& other) const {
			return m_Pmax == other.m_Pmax
				&& m_Pmin == other.m_Pmin
				;
		}
	public:
		BBox() : m_Pmin(glm::vec3(inf, inf, inf)), m_Pmax(glm::vec3(-inf, -inf, -inf)) {}
		BBox(const glm::vec3& p0, const glm::vec3& p1) {
			m_Pmin = Min(p0, p1);
			m_Pmax = Max(p0, p1);
		}

		// Expand current BBox to include another BBox
		void Merge(const BBox& other) {
			m_Pmin = Min(m_Pmin, other.m_Pmin);
			m_Pmax = Max(m_Pmax, other.m_Pmax);
		}

		void Merge(const glm::vec3& p) {
			m_Pmin = Min(m_Pmin, p);
			m_Pmax = Max(m_Pmax, p);
		}

		static BBox Merge(const BBox& lhs, const BBox& rhs);
		static BBox Merge(const BBox& lhs, const glm::vec3& p);

		bool Overlaps(const BBox& other) {
			return false;
		}

		glm::vec3 Offset(const glm::vec3& p) const {
			glm::vec3 o = p - m_Pmin;
			if (m_Pmax.x > m_Pmin.x) o.x /= m_Pmax.x - m_Pmin.x;
			if (m_Pmax.y > m_Pmin.y) o.y /= m_Pmax.y - m_Pmin.y;
			if (m_Pmax.z > m_Pmin.z) o.z /= m_Pmax.z - m_Pmin.z;
			AHO_CORE_ASSERT(o.x >= 0.0f && o.y >= 0.0f && o.z >= 0.0f);
			AHO_CORE_ASSERT(o.x <= 1.0f && o.y <= 1.0f && o.z <= 1.0f);

			return o;
		}

		bool Intersect(const Ray& ray) const;

		bool IntersectNearest(const Ray& ray, float& t) const;

		glm::vec3 GetCentroid() const { return glm::vec3(0.5) * m_Pmin + glm::vec3(0.5) * m_Pmax; }

		// Diagonal
		glm::vec3 GetAbsDiff() const {
			glm::vec3 d = m_Pmax - m_Pmin;
			AHO_CORE_ASSERT(d.x >= 0 && d.y >= 0 && d.z >= 0);
			return d;
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
		glm::vec3 m_Pmin; float _padding0;
		glm::vec3 m_Pmax; float _padding1;
	};
}