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