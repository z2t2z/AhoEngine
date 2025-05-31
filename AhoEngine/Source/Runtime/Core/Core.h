#pragma once
#include <filesystem> 
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#ifdef AHO_PLATFORM_WINDOWS
	#ifdef AHO_DYNAMIC_LINK
		#ifdef AHO_BUILD_DLL
			#define AHO_API __declspec(dllexport)
		#else
			#define AHO_API __declspec(dllimport)
		#endif
	#else
		#define AHO_API 
	#endif
#else 
	#error You are AHO!
#endif

// Some useful macros
#define BIT(x) (1 << x)
#define AHO_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)


#define AHO_EXPAND_MACRO(x) x
#define AHO_STRINGIFY_MACRO(x) #x

#define AHO_DEBUGBREAK() __debugbreak()


// Assertions
#ifdef AHO_ENABLE_ASSERTS
	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define AHO_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { AHO##type##ERROR(msg, __VA_ARGS__); AHO_DEBUGBREAK(); } }
	#define AHO_INTERNAL_ASSERT_WITH_MSG(type, check, ...) AHO_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define AHO_INTERNAL_ASSERT_NO_MSG(type, check) AHO_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", AHO_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define AHO_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define AHO_INTERNAL_ASSERT_GET_MACRO(...) AHO_EXPAND_MACRO( AHO_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, AHO_INTERNAL_ASSERT_WITH_MSG, AHO_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define AHO_ASSERT(...) AHO_EXPAND_MACRO( AHO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define AHO_CORE_ASSERT(...) AHO_EXPAND_MACRO( AHO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define AHO_ASSERT(x, ...)
	#define AHO_CORE_ASSERT(x, ...)
#endif // AHO_ENABLE_ASSERTS
