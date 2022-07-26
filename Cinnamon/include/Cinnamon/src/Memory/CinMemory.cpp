#include "Cinnamon/include/Memory/CinMemory.h"

namespace Cinnamon {
	InternalScope AllocatorData s_AllocatorData;
	//GlobalAllocator::AllocateProxy GlobalAllocator::Allocate(const std::size_t elementCount, const char* file, const uint32_t line)
	//{
	//	return AllocateProxy(elementCount, file, line);
	//}

	void GlobalAllocator::Deallocate(void* const userPointer)
	{
#if CIN_TRACK_MEMORY 
		{
			std::lock_guard<std::mutex> lock{ s_AllocatorData.AllocationTrackerInsertMutex };
			s_AllocatorData.Allocations.erase(userPointer);
		}
#endif
		operator delete(userPointer);
	}

#if CIN_TRACK_MEMORY 
	void GlobalAllocator::DumpAllocations()
	{
		CIN_INFO("Dumpimg memory");
		{
			std::lock_guard<std::mutex> lock{ s_AllocatorData.AllocationTrackerInsertMutex };
			for (const auto& [userPointer, allocationData] : s_AllocatorData.Allocations) {
				CIN_WARN("Unfreed memory at {0}, requested by {1}, {2}", userPointer, allocationData.File, allocationData.Line);
                CIN_UNUSED(userPointer);
                CIN_UNUSED(allocationData);
            }
		}
		CIN_INFO("Finished dumping memory");
	}

	AllocatorData& GlobalAllocator::GetAllocatorData()
	{
		return s_AllocatorData;
	}
#endif
}