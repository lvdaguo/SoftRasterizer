#include "pch.h"
#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void Logger::Init()
{
	if (AllocConsole())
	{
		freopen_s(&m_file, "CONOUT$", "w", stdout);

		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(CreateRef<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(CreateRef<spdlog::sinks::basic_file_sink_mt>("SoftRasterizer.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		m_logger = CreateRef<spdlog::logger>("RST", begin(logSinks), end(logSinks));
		spdlog::register_logger(m_logger);
		m_logger->set_level(spdlog::level::trace);
		m_logger->flush_on(spdlog::level::trace);

		LOG_TRACE("init logger");
	}
	else
	{
		TIPS(L"failed to create consloe");
	}
}

Logger::~Logger()
{
	fclose(stdout);
}
