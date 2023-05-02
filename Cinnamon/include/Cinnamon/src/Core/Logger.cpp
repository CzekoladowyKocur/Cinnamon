#include "Cinnamon/include/Core/Logger.hpp"
	
namespace Cinnamon {
	ELogLevel Logger::s_LogLevel{ ELogLevel::Trace };

	bool Logger::Initialize(const ELogLevel logLevel)
	{
		CIN_ASSERT(logLevel >= ELogLevel::Begin && logLevel <= ELogLevel::End);
		s_LogLevel = logLevel;

		return true;
	}

	bool Logger::Shutdown()
	{
		return true;
	}
}