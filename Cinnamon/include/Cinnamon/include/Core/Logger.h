#pragma once
#include "Cinnamon/include/Core/Core.h"
#include <format>
#include <iostream>
#include <string_view>
#include <chrono>

namespace Cinnamon {
	enum class ELogLevel
	{
		Trace = 0,
		Info,
		Warn,
		Error,
		Critical,
		Begin = Trace,
		End = Critical,
	};

	class Logger
	{
	public:
	public:
		Logger() noexcept = delete;
		~Logger() noexcept = delete;

		static void Initialize(const ELogLevel logLevel);
		static void Shutdown();

		template<typename ... Args>
		static constexpr void Log(const ELogLevel logLevel, const std::string_view message, Args&& ... args)
		{
			/* TODO: Support for colors and file output */
			FunctionVariable const char* s_LogLevelLabels[5]{ "TRACE: ", "INFO: ", "WARN: ", "ERROR: ", "CRITICAL: " };
			if (logLevel < s_LogLevel || logLevel < ELogLevel::Begin || logLevel > ELogLevel::End)
				return;

			const std::string formatted{ std::vformat(message, std::make_format_args(std::forward<Args>(args)...)) };
			std::string prefix{ std::format("[{0}] ", std::chrono::system_clock::now()) };
			prefix.append(s_LogLevelLabels[static_cast<std::size_t>(logLevel)]);
			prefix.append(formatted);
		
			std::cout << prefix << '\n';
		}
	private:
		static ELogLevel s_LogLevel;
	};
}

/* TODO: Make seperate engine core and application logging macros */
#define CIN_DISABLE_LOGGING 0
#if CIN_DISABLE_LOGGING
#define CIN_TRACE(message, ...)				void()
#define CIN_INFO(message, ...)				void()
#define CIN_WARN(message, ...)				void()
#define CIN_ERROR(message, ...)				void()
#define CIN_CRITICAL(message, ...)			void()
#else
#define CIN_TRACE(message, ...)				Logger::Log(ELogLevel::Trace, message, ##__VA_ARGS__)
#define CIN_INFO(message, ...)				Logger::Log(ELogLevel::Info, message, ##__VA_ARGS__)
#define CIN_WARN(message, ...)				Logger::Log(ELogLevel::Warn, message, ##__VA_ARGS__)
#define CIN_ERROR(message, ...)				Logger::Log(ELogLevel::Error, message, ##__VA_ARGS__)
#define CIN_CRITICAL(message, ...)			Logger::Log(ELogLevel::Critical, message, ##__VA_ARGS__)
#endif