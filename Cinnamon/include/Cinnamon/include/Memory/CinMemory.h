#pragma once
#include "Cinnamon/include/Core/Core.h"
/*
	L1 cache reference                            0.5 ns
	Branch mispredict                             5   ns
	L2 cache reference                            7   ns           14x L1 cache
	Mutex lock/unlock                            25   ns
	Main memory reference                       100   ns           20x L2 cache, 200x L1 cache
	Compress 1K bytes with Zippy              3,000   ns
	Send 1K bytes over 1 Gbps network        10,000   ns  0.01 ms
	Read 4K randomly from SSD*              150,000   ns  0.15 ms
	Read 1 MB sequentially from memory      250,000   ns  0.25 ms
	Round trip within same datacenter       500,000   ns  0.5  ms
	Read 1 MB sequentially from SSD*      1,000,000   ns  1    ms  4X memory
	Disk seek                            10,000,000   ns  10   ms  20x datacenter roundtrip
	Read 1 MB sequentially from disk     20,000,000   ns  20   ms  80x memory, 20X SSD
	Send packet CA->Netherlands->CA     150,000,000   ns  150  ms
	LESSON LEARNED: Manage your memory kids
*/

/*
	On Windows and OSX, the size of a page is 4 kilobytes.
	On console platforms, the size of a page is generally 4 kilobytes.
	On iOS, the size of a page is 16 kilobytes (also backed by 16 kilobyte physical pages on A8 systems).

	Paging affects performance negatively, and should avoided on desktop platforms.
	- Allocations smaller than the size of a virtual memory page cannot be page-aligned.
	- Large allocations are guaranteed to be page-aligned.
*/

/*
	Write-combined memory is a type of non-cacheable memory where writes bypass the CPU caches and are written directly to main memory.
	Useful for data destined for the GPU that will not be read by the CPU.
*/


/*
scripted objects usually work with handles instead of pointers to support hot-reload.
*/
#define SIZE_KB 1024
#define SIZE_MB 1024 * 1024
#define SIZE_GB 1024 * 1024

namespace Cinnamon {
	//bool IsAddressPowerOfTwo(const std::uintptr_t n)
	//{
	//	return (n != 0) && ((n & (n - 1)) == 0);
	//}

	//void* Align(const std::uintptr_t ptr, const std::size_t alignment)
	//{
	//	return reinterpret_cast<void*>(((ptr + (alignment - 1)) & -((int)alignment)));
	//}

	class GlobalAllocator
	{
	public:
		static auto* Allocate(const std::size_t size)
		{
			return malloc(size);		
		}

		static void Deallocate(void* const ptr)
		{
			return free(ptr);
		}
	private:
	};

#define CIN_NEW(size) GlobalAllocator::Allocate(size);
#define CIN_DELETE(ptr)  GlobalAllocator::Deallocate(ptr);
}