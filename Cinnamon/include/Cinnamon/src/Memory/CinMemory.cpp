#include "Cinnamon/include/Memory/CinMemory.h"

namespace Cinnamon {
	void DumpCallbackFunction(const void* allocation, const char* typeName, const std::size_t allocationSize, const char* filename, const std::size_t fileLine) noexcept
	{
		CIN_WARN("[Thread: {0}] Found unfreed memory at {1} ({2} bytes) of type {3} [{4}, {5}]", "TODO", allocation, allocationSize,typeName, filename, fileLine);
	}
}