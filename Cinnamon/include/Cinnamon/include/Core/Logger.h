#pragma once
#include "Cinnamon/include/Core/Core.h"

/* Disable fmt warnings */
#ifdef CIN_PLATFORM_WINDOWS
#pragma warning(push, 0)
#elif defined CIN_PLATFORM_LINUX
/* no */
#endif

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/chrono.h"

#ifdef CIN_PLATFORM_WINDOWS
#pragma warning(pop)
#elif defined CIN_PLATFORM_LINUX
/* no */
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
			if (logLevel < s_LogLevel)
				return;

			// thread_local FunctionVariable char preallocatedBuffer[512];
			STL::String formatted;
			formatted.resize(512);

			formatted = std::move(fmt::format("[{0}]", std::chrono::system_clock::now()));
			formatted.append(fmt::vformat(message, fmt::make_format_args(std::forward<Args>(args)...)));
			Output<logLevel>(formatted.c_str());
		}

		template<ELogLevel logLevel>
		static void Output(const char* message);
	private:
		static ELogLevel s_LogLevel;
	};
	template<>
	inline void Logger::Output<ELogLevel::Trace>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Gray);
	}

	template<>
	inline void Logger::Output<ELogLevel::Info>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::White);
	}

	template<>
	inline void Logger::Output<ELogLevel::Warn>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Yellow);
	}

	template<>
	inline void Logger::Output<ELogLevel::Error>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Red);
	}

	template<>
	inline void Logger::Output<ELogLevel::Critical>(const char* message)
	{
		Platform::WriteToConsole(message, EConsoleTextColor::Magenta);
	}
}

/* TODO: Make seperate engine core and application logging macros */
#ifdef CIN_DISTRIBUTION 
#define CIN_DISABLE_LOGGING 1U
#else
#define CIN_DISABLE_LOGGING 0U
#endif

#if CIN_DISABLE_LOGGING
#define CIN_TRACE(message, ...)             void()
#define CIN_INFO(message, ...)              void()
#define CIN_WARN(message, ...)              void()
#define CIN_ERROR(message, ...)             void()
#define CIN_CRITICAL(message, ...)          void()
#else
#define CIN_TRACE(message, ...)             Logger::Log<ELogLevel::Trace>(" [TRACE] " message "\n", ##__VA_ARGS__)
#define CIN_INFO(message, ...)              Logger::Log<ELogLevel::Info>(" [INFO] " message "\n", ##__VA_ARGS__)
#define CIN_WARN(message, ...)              Logger::Log<ELogLevel::Warn>(" [WARN] " message "\n", ##__VA_ARGS__)
#define CIN_ERROR(message, ...)             Logger::Log<ELogLevel::Error>(" [ERROR] " message "\n", ##__VA_ARGS__)
#define CIN_CRITICAL(message, ...)          Logger::Log<ELogLevel::Critical>(" [CRITICAL] " message "\n", ##__VA_ARGS__)
#endif
