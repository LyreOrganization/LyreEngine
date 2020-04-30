#pragma once

void start_logging();

void stop_logging();

enum class LogLevel : int
{
	FATAL = 0,
	ERROR,
	WARNING,
	INFO
};

void log_msg(LogLevel level, const std::string& msg);

void log_msg(const std::string& msg);

#ifdef DISABLE_LOG
#define LOG_START() do {} while(0)
#define LOG_STOP() do {} while(0)
#define LOG_MSG(msg) do {} while(0)
#define LOG_MSG(level, msg) do {} while(0)
#else
#define LOG_START() do { start_logging(); } while(0)
#define LOG_STOP() do { stop_logging(); } while(0)
#define LOG_MSG(level, msg) do { log_msg(level, msg); } while(0)
#endif

#define LOG_FATAL(msg) LOG_MSG(LogLevel::FATAL, msg)
#define LOG_ERR(msg) LOG_MSG(LogLevel::ERROR, msg)
#define LOG_WARN(msg) LOG_MSG(LogLevel::WARNING, msg)
#define LOG_INFO(msg) LOG_MSG(LogLevel::INFO, msg)