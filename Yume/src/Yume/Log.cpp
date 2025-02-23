#include "ympch.h"

#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Yume 
{	
	std::shared_ptr<spdlog::logger> Log::s_ymLogger;
	std::shared_ptr<spdlog::logger> Log::s_appLogger;

	void Log::init()
	{	
		// Timestamp Name: Message
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_ymLogger = spdlog::stdout_color_mt("YUME");
		s_ymLogger->set_level(spdlog::level::trace);

		s_appLogger = spdlog::stdout_color_mt("APP");
		s_appLogger->set_level(spdlog::level::trace);
	}

}
