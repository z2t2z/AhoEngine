#pragma once

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <ranges>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <execution>
#include <chrono>
#include <random>
#include <thread>
#include <future>
#include <mutex>
#include "Runtime/Core/Log/Log.h"

#ifdef AHO_PLATFORM_WINDOWS
	#include <Windows.h>
#endif