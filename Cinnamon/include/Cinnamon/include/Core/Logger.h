#pragma once
#include "Cinnamon/include/Core/Core.h"
#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/chrono.h"
#include "fmt/core.h"

#ifdef CIN_PLATFORM_LINUX
#include <any>
#endif


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

		static bool Initialize(const ELogLevel logLevel);
		static bool Shutdown();

		template<ELogLevel logLevel, typename ... Args>
		static constexpr void Log(const fmt::string_view message, Args&& ... args)
		{
#if CIN_PLATFORM_WINDOWS
			if (logLevel < s_LogLevel)
				return;

			// thread_local FunctionVariable char preallocatedBuffer[512];
			STL::String formatted;
			formatted.resize(512);

			formatted = std::move(fmt::format("[{0}]", std::chrono::system_clock::now()));
			formatted.append(fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
			Output<logLevel>(formatted.c_str());
#endif
		}

		template<ELogLevel logLevel>
		static void Output(const char* message);
	private:
		static ELogLevel s_LogLevel;
	};
    #ifdef CIN_PLATFORM_WINDOWS
	template<>
	static inline void Logger::Output<ELogLevel::Trace>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Gray);
	}

	template<>
	static inline void Logger::Output<ELogLevel::Info>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::White);
	}

	template<>
	static inline void Logger::Output<ELogLevel::Warn>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Yellow);
	}

	template<>
	static inline void Logger::Output<ELogLevel::Error>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Red);
	}

	template<>
	static inline void Logger::Output<ELogLevel::Critical>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Magenta);
	}
    #endif
}

/* TODO: Make seperate engine core and application logging macros */
#define CIN_DISABLE_LOGGING CIN_PLATFORM_LINUX
#if CIN_DISABLE_LOGGING
#define CIN_TRACE(message, ...)				void()
#define CIN_INFO(message, ...)				void()
#define CIN_WARN(message, ...)				void()
#define CIN_ERROR(message, ...)				void()
#define CIN_CRITICAL(message, ...)			void()
#else
#define CIN_TRACE(message, ...)				Logger::Log<ELogLevel::Trace>(" [TRACE] "##message##"\n", ##__VA_ARGS__)
#define CIN_INFO(message, ...)				Logger::Log<ELogLevel::Info>(" [INFO] "##message##"\n", ##__VA_ARGS__)
#define CIN_WARN(message, ...)				Logger::Log<ELogLevel::Warn>(" [WARN] "##message##"\n", ##__VA_ARGS__)
#define CIN_ERROR(message, ...)				Logger::Log<ELogLevel::Error>(" [ERROR] "##message##"\n", ##__VA_ARGS__)
#define CIN_CRITICAL(message, ...)			Logger::Log<ELogLevel::Critical>(" [CRITICAL] "##message##"\n", ##__VA_ARGS__)
#endif
