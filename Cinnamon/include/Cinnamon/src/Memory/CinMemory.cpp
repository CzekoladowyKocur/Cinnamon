#include "Cinnamon/include/Memory/CinMemory.hpp"

namespace Cinnamon {
	void DumpCallbackFunction(const void* allocation, const char* typeName, const std::size_t allocationSize, const char* filename, const std::size_t fileLine) noexcept
	{
#if CIN_DISABLE_LOGGING
		CIN_UNUSED(allocation);
		CIN_UNUSED(typeName);
		CIN_UNUSED(allocationSize);
		CIN_UNUSED(filename);
		CIN_UNUSED(fileLine);
#else
		CIN_WARN("[Thread: {0}] Found unfreed memory at {1} ({2} bytes) of type {3} [{4}, {5}]", "TODO", allocation, allocationSize, typeName, filename, fileLine);
#endif
	}
}