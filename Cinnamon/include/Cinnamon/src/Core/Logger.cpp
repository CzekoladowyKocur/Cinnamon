#include "Cinnamon/include/Core/Logger.h"
	
namespace Cinnamon {
	ELogLevel Logger::s_LogLevel{ ELogLevel::Trace };

	void Logger::Initialize(const ELogLevel logLevel)
	{
		CIN_ASSERT(logLevel > ELogLevel::Begin && logLevel < ELogLevel::End);
		s_LogLevel = logLevel;
	}

	void Logger::Shutdown()
	{}
}