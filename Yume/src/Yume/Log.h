#pragma once

#include<memory>

#include "API.h"
#include "spdlog/spdlog.h"

namespace Yume {

	class YM_API Log
	{
	public:
		static void init();

		inline static std::shared_ptr<spdlog::logger>& getYMLogger() { return m_ymLogger; }
		inline static std::shared_ptr<spdlog::logger>& getAppLogger() { return m_appLogger; }

	private:
		static std::shared_ptr<spdlog::logger> m_ymLogger;
		static std::shared_ptr<spdlog::logger> m_appLogger;
	};

}

// Yume log macros.
#define YM_TRACE(...)			::Yume::Log::getYMLogger()->trace(__VA_ARGS__)
#define YM_INFO(...)			::Yume::Log::getYMLogger()->info(__VA_ARGS__)
#define YM_WARN(...)			::Yume::Log::getYMLogger()->warn(__VA_ARGS__)
#define YM_ERROR(...)			::Yume::Log::getYMLogger()->error(__VA_ARGS__)
#define YM_FATAL(...)			::Yume::Log::getYMLogger()->fatal(__VA_ARGS__)

// App log macros
#define YM_APP_TRACE(...)		::Yume::Log::getAppLogger()->trace(__VA_ARGS__)
#define YM_APP_INFO(...)		::Yume::Log::getAppLogger()->info(__VA_ARGS__)
#define YM_APP_WARN(...)		::Yume::Log::getAppLogger()->warn(__VA_ARGS__)
#define YM_APP_ERROR(...)		::Yume::Log::getAppLogger()->error(__VA_ARGS__)
#define YM_APP_FATAL(...)		::Yume::Log::getAppLogger()->fatal(__VA_ARGS__)


