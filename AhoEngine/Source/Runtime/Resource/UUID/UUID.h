#pragma once

#include <cstdint>
#include <unordered_map>

namespace Aho {
	class UUID {
	public:
		UUID();
		UUID(uint64_t);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<Aho::UUID> {
		std::size_t operator()(const Aho::UUID& uuid) const {
			return (uint64_t)uuid;
		}
	};

}