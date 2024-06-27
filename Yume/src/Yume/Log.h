#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"

namespace Yume {

	class YM_API Log
	{
	public:
		static void init();

		static std::shared_ptr<spdlog::logger>& getYMLogger() { return s_ymLogger; }
		static std::shared_ptr<spdlog::logger>& getAppLogger() { return s_appLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_ymLogger;
		static std::shared_ptr<spdlog::logger> s_appLogger;
	};

}

// Yume log macros.
#define YM_CORE_TRACE(...)			::Yume::Log::getYMLogger()->trace(__VA_ARGS__)
#define YM_CORE_INFO(...)			::Yume::Log::getYMLogger()->info(__VA_ARGS__)
#define YM_CORE_WARN(...)			::Yume::Log::getYMLogger()->warn(__VA_ARGS__)
#define YM_CORE_ERROR(...)			::Yume::Log::getYMLogger()->error(__VA_ARGS__)
#define YM_CORE_FATAL(...)			::Yume::Log::getYMLogger()->critical(__VA_ARGS__)

// App log macros
#define YM_TRACE(...)		::Yume::Log::getAppLogger()->trace(__VA_ARGS__)
#define YM_INFO(...)		::Yume::Log::getAppLogger()->info(__VA_ARGS__)
#define YM_WARN(...)		::Yume::Log::getAppLogger()->warn(__VA_ARGS__)
#define YM_ERROR(...)		::Yume::Log::getAppLogger()->error(__VA_ARGS__)
#define YM_FATAL(...)		::Yume::Log::getAppLogger()->critical(__VA_ARGS__)


