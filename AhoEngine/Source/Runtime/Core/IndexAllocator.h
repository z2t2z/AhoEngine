#pragma once

#include <stack>
#include <unordered_map>

namespace Aho {
	class IndexAllocator {
	public:
		template <typename T>
		uint32_t AcquireIndex() {
			uint32_t typeID = GetTypeID<T>();
			auto& stack = m_FreeIDs[typeID];
			if (!stack.empty()) {
				uint32_t id = stack.top();
				stack.pop();
				return id;
			}
			return m_IDs[typeID]++;
		}
		template <typename T>
		void ReleaseIndex(uint32_t index) {
			uint32_t typeId = GetTypeID<T>();
			m_FreeIDs[typeId].push(index);
		}
	private:
		template <typename T>
		uint32_t GetTypeID() {
			static const uint32_t id = s_GlobalTypeCounter++;
			return id;
		}
	private:
		std::unordered_map<uint32_t, std::stack<uint32_t>> m_FreeIDs;
		std::unordered_map<uint32_t, uint32_t> m_IDs;
		inline static uint32_t s_GlobalTypeCounter = 0;
	};
}
