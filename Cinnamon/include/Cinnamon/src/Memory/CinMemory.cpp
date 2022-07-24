#include "Cinnamon/include/Memory/CinMemory.h"
#include <list>
namespace Cinnamon {
	struct {
		void* Buffer;
		uint32_t Pointer;

	} static s_AllocatorData;

	GlobalAllocator::AllocateProxy GlobalAllocator::Allocate(const std::size_t elementCount, const char* file, const uint32_t line)
	{
		return AllocateProxy(elementCount, file, line);
	}

	void GlobalAllocator::Deallocate(void* const ptr)
	{
		operator delete(ptr);
	}
}