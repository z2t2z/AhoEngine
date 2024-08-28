#pragma once


#include <memory>
#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

// https://github.com/gabime/spdlog


// Core Logger Macro
// For distribution build, just replace with something else so that we won't lose performance
#define AHO_CORE_ERROR(...) ::Aho::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AHO_CORE_WARN(...)  ::Aho::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AHO_CORE_INFO(...)  ::Aho::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AHO_CORE_TRACE(...) ::Aho::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AHO_CORE_FATAL(...) ::Aho::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client Logger Macro
#define AHO_ERROR(...) ::Aho::Log::GetClientLogger()->error(__VA_ARGS__)
#define AHO_WARN(...)  ::Aho::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AHO_INFO(...)  ::Aho::Log::GetClientLogger()->info(__VA_ARGS__)
#define AHO_TRACE(...) ::Aho::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AHO_FATAL(...) ::Aho::Log::GetClientLogger()->fatal(__VA_ARGS__)



namespace Aho {

	class AHO_API Log {

	public:
		Log();

		~Log();

		static void Init();

		// Two logger for client and our engine seperately
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
			return s_CoreLogger;
		}

		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {
			return s_ClientLogger;
		}

	private:

		static std::shared_ptr<spdlog::logger> s_CoreLogger;

		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

}

