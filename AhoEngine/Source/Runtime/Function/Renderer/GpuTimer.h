#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>

namespace Aho {
	class GpuTimer {
	public:
		GpuTimer(int queryLatency = 4)
			: m_QueryLatency(queryLatency), m_QueryIndex(0), m_FrameCount(0) {
			m_Queries.resize(queryLatency);
			glGenQueries(queryLatency, m_Queries.data());
			m_Timestamps.resize(queryLatency, 0.0);
			m_Used.resize(queryLatency, false);
		}

		~GpuTimer() {
			glDeleteQueries(static_cast<GLsizei>(m_Queries.size()), m_Queries.data());
		}

		void Begin() {
			m_QueryIndex = m_FrameCount % m_QueryLatency;
			glBeginQuery(GL_TIME_ELAPSED, m_Queries[m_QueryIndex]);
			m_Used[m_QueryIndex] = true;
		}

		void End() {
			glEndQuery(GL_TIME_ELAPSED);
			++m_FrameCount;
		}

		// 在 Begin/End 后的几帧调用
		void Update() {
			int readIndex = (m_FrameCount + 1) % m_QueryLatency;
			if (!m_Used[readIndex]) return;

			GLint available = 0;
			glGetQueryObjectiv(m_Queries[readIndex], GL_QUERY_RESULT_AVAILABLE, &available);
			if (available) {
				GLuint64 timeNs = 0;
				glGetQueryObjectui64v(m_Queries[readIndex], GL_QUERY_RESULT, &timeNs);
				m_Timestamps[readIndex] = timeNs / 1e6; // Convert ns → ms
			}
		}

		// 获取最新一次读到的有效时间（ms）
		double GetLatestTime() const {
			int latestIndex = (m_FrameCount + 1) % m_QueryLatency;
			return m_Timestamps[latestIndex];
		}

	private:
		int m_QueryLatency;
		int m_QueryIndex;
		uint64_t m_FrameCount;

		std::vector<GLuint> m_Queries;
		std::vector<double> m_Timestamps;
		std::vector<bool> m_Used;
	};

}
