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
#include <chrono>
#include <random>
#include "Runtime/Core/Log/Log.h"

#ifdef AHO_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // !IMGUI_DEFINE_MATH_OPERATORS
