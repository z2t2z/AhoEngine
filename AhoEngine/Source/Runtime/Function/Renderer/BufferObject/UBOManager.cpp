#include "Ahopch.h"
#include "UBOManager.h"

namespace Aho {
	std::unordered_map<uint32_t, std::unique_ptr<UBOBase>> UBOManager::m_UBOs;
}