#pragma once

#ifdef AHO_PLATFORM_WINDOWS
	#ifdef AHO_BUILD_DLL
		#define AHO_API __declspec(dllexport)
	#else
		#define AHO_API __declspec(dllimport)
	#endif
#else
	#error You are AHO!
#endif


#define BIT(x) (1 << x)


// Assertions
#ifdef AHO_ENABLE_ASSERTS
	#define AHO_ASSERT(x, ...) {if (!x) { HZ_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define AHO_CORE_ASSERT(x, ...) {if (!x) { HZ_CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define AHO_ASSERT(x, ...)
	#define AHO_CORE_ASSERT(x, ...)
#endif // AHO_ENABLE_ASSERTS
