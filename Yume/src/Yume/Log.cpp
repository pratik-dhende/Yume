#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Yume 
{	
	std::shared_ptr<spdlog::logger> Log::mYMLogger;
	std::shared_ptr<spdlog::logger> Log::mAppLogger;

	void Log::init()
	{	
		// Timestamp Name: Message
		spdlog::set_pattern("%^[%T] %n: %v%$");

		mYMLogger = spdlog::stdout_color_mt("YUME");
		mYMLogger->set_level(spdlog::level::trace);

		mAppLogger = spdlog::stdout_color_mt("APP");
		mAppLogger->set_level(spdlog::level::trace);
	}

}
