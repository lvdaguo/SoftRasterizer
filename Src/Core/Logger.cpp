#include "pch.h"
#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

static const unsigned int CONSOLE_WINDOW_WIDTH = 600;
static const unsigned int CONSOLE_WINDOW_HEIGHT = 600;

std::ostream& operator << (std::ostream& out, const vec2& v)
{
	out << "[" << v.x << ", " << v.y << "]";
	return out;
}

std::ostream& operator << (std::ostream& out, const vec3& v)
{
	out << "[" << v.x << ", " << v.y << ", " << v.z << "]";
	return out;
}

std::ostream& operator << (std::ostream& out, const vec4& v)
{
	out << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
	return out;
}

void Logger::Init()
{
	// 创建控制台
	if (AllocConsole())
	{
		freopen_s(&m_file, "CONOUT$", "w", stdout);

		// 移动控制台窗口位置到右上角 
		HWND console = GetConsoleWindow();
		RECT rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		int screenW = rect.right - rect.left;
		int screenH = rect.bottom - rect.top;

		int width = CONSOLE_WINDOW_WIDTH;
		int height = CONSOLE_WINDOW_HEIGHT;
		MoveWindow(console, screenW - width, 0, width, height, TRUE);

		// 初始化logger
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
