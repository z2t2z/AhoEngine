#include "Ahopch.h"
#include "FileWatcher.h"


namespace Aho {
	template <>
	std::unique_ptr<FileWatcher> SingletonBase<FileWatcher>::m_Instance = nullptr;
}
