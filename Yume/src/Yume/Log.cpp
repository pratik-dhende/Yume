#include "ympch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Yume 
{	
	std::shared_ptr<spdlog::logger> Log::m_ymLogger;
	std::shared_ptr<spdlog::logger> Log::m_appLogger;

	void Log::init()
	{	
		// Timestamp Name: Message
		spdlog::set_pattern("%^[%T] %n: %v%$");

		m_ymLogger = spdlog::stdout_color_mt("YUME");
		m_ymLogger->set_level(spdlog::level::trace);

		m_appLogger = spdlog::stdout_color_mt("APP");
		m_appLogger->set_level(spdlog::level::trace);
	}

}
