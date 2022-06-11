#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

class Logger : public Singleton<Logger>
{
public:
	void Init();
	virtual ~Logger();

public:
	Ref<spdlog::logger> GetLogger() { return m_logger; }

private:
	FILE* m_file;
	Ref<spdlog::logger> m_logger;
};

// ����vec��log�����ʽ
std::ostream& operator << (std::ostream& out, const vec2& v);
std::ostream& operator << (std::ostream& out, const vec3& v);
std::ostream& operator << (std::ostream& out, const vec4& v);

#define LOG_ENABLED

#ifdef LOG_ENABLED
#define LOG_TRACE(...)       Logger::Instance().GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)        Logger::Instance().GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)        Logger::Instance().GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)       Logger::Instance().GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)    Logger::Instance().GetLogger()->critical(__VA_ARGS__)
#else
#define LOG_TRACE(...)       
#define LOG_INFO(...)        
#define LOG_WARN(...)        
#define LOG_ERROR(...)       
#define LOG_CRITICAL(...)    
#endif