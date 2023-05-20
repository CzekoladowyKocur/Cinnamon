#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include <typeinfo>
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

#define SIZE_KB (1024U)
#define SIZE_MB (SIZE_KB * 1024U) 
#define SIZE_GB (SIZE_MB * 1024U) 
#define KILOBYTES(count) (1024U * count)
#define MEGABYTES(count) (KILOBYTES * 1024U * count)
#define GIGABYTES(count) (MEGABYTES * 1024U * count)

#define CIN_ALLOCATOR_FORCE_INLINE	CIN_FORCE_INLINE

#define CIN_ALLOCATOR_SHARED_STATE 1 /* Can free allocated memory from all threads */ 
#define CIN_ALLOCATOR_USE_NOTHROW_NEW 0 /* Disable exceptions */
#if CIN_ALLOCATOR_USE_NOTHROW_NEW 
#define CIN_ALLOCATOR_THROW_ATTRIBUTE (std::nothrow)
#else
#define CIN_ALLOCATOR_THROW_ATTRIBUTE
#endif

namespace Cinnamon {
	void DumpCallbackFunction(const void* allocation, const char* typeName, const std::size_t allocationSize, const char* filename, const std::size_t fileLine) noexcept;
#if CIN_ALLOCATOR_SHARED_STATE
	class ThreadAllocatorData final
#else
	thread_local class ThreadAllocatorData final
#endif
	{
	public:
		struct AllocationData
		{
			const char* Name;
			std::size_t Size;
			const char* File;
			std::size_t Line;

			explicit AllocationData(const char* name, const std::size_t size, const char* file, const std::size_t line) noexcept
				:
				Name(name),
				Size(size),
				File(file),
				Line(line)
			{}
		};
#if CIN_ALLOCATOR_SHARED_STATE
		static inline std::unordered_map<const void*, AllocationData> Allocations;
		static inline std::atomic<bool> s_MainThreadExited{ false };
		static inline std::mutex s_AllocationRegistryMutex;
#else
		std::unordered_map<const void*, AllocationData> Allocations;
#endif
	public:
		~ThreadAllocatorData() noexcept
		{
			if (!s_MainThreadExited)
				DumpThreadMemory();
		}

		CIN_ALLOCATOR_FORCE_INLINE void DumpThreadMemory()
		{
#if CIN_ALLOCATOR_SHARED_STATE
			const std::lock_guard<std::mutex> lock(s_AllocationRegistryMutex);
			/* Sleep for 200 to let other threads clean up, still might fire false positives... */
			//const std::chrono::duration<int, std::milli> sleepDuration{ 200 };
			//std::this_thread::sleep_for(sleepDuration);
#endif
			if (!Allocations.empty())
			{
				for (const auto& [address, allocation] : Allocations)
					DumpCallbackFunction(address, allocation.Name, allocation.Size, allocation.File, allocation.Line);
			}
		}

		CIN_ALLOCATOR_FORCE_INLINE void Register(const void* address, const char* name, const std::size_t size, const char* file, const std::size_t line) noexcept
		{
#if CIN_ALLOCATOR_SHARED_STATE
			const std::lock_guard<std::mutex> lock(s_AllocationRegistryMutex);
#endif
			Allocations.emplace(address, std::move(AllocationData{ name, size, file, line }));
		}

		CIN_ALLOCATOR_FORCE_INLINE void Unregister(const void* address) noexcept
		{
#if CIN_ALLOCATOR_SHARED_STATE
			const std::lock_guard<std::mutex> lock(s_AllocationRegistryMutex);
#endif
			CIN_ASSERT(Allocations.find(address) != Allocations.end(), "Allocation came from a different thread (invalid if shared state is disabled) or was not done via the API");
			Allocations.erase(address);
		}
	} static t_ThreadAllocatorData;

	static inline void CinDumpThreadMemory()
	{
		t_ThreadAllocatorData.DumpThreadMemory();
	}

	struct AllocationProxy final
	{
		const char* File;
		const uint32_t Line;

		explicit constexpr AllocationProxy(const char* file, const uint32_t line) noexcept
			:
			File(file),
			Line(line)
		{}

		~AllocationProxy() noexcept = default;

		template <typename T>
		[[nodiscard]] CIN_FORCE_INLINE T* operator << (T* const allocation) const noexcept
		{
			/* Forward the allocation to variable */
			t_ThreadAllocatorData.Register(allocation, typeid(T).name(), sizeof(T), File, Line);
			return allocation;
		}
	};

	struct DeallocationProxy final
	{
		explicit constexpr DeallocationProxy() noexcept = default;
		~DeallocationProxy() noexcept = default;

		template<typename T>
		CIN_FORCE_INLINE void operator << (T* const address) noexcept
		{
			t_ThreadAllocatorData.Unregister(address);
			delete address;
		}
	};

	struct DeallocationProxyArray final
	{
		explicit constexpr DeallocationProxyArray() noexcept = default;
		~DeallocationProxyArray() noexcept = default;

		template<typename T>
		CIN_FORCE_INLINE void operator << (T* const address) noexcept
		{
			t_ThreadAllocatorData.Unregister(address);
			delete[] address;
		}
	};
	/*
	* When the compiler sees a _new_ expression, it first deduces the appropriate overloaded
	* new operator (::operator new(...)), depending on the number and type of passed arguments, then,
	* the function is invoked, returning a pointer to raw block of memory.
	*
	* new expression:
	* int* integer = new int;
	* new operator:
	* void* storagePointer = ::operator new(sizeof (int));
	* ... and using new expression:
	* int* integer = new (storagePointer) int;
	* Note that _new_ expression performs no allocation.
	*/
#if (false)
	/* If exceptions are disabled, return value should be checked (no different from standard new) */
#define cinew AllocationProxy(__FILE__, __LINE__) << new CIN_ALLOCATOR_THROW_ATTRIBUTE
#define cindel DeallocationProxy() << 
#define cindelarr DeallocationProxyArray() << 
#define CIN_DUMP_ALLOCATIONS CinDumpThreadMemory
#else
	/* If exceptions are disabled, return value should be checked (no different from standard new) */
#define cinew new CIN_ALLOCATOR_THROW_ATTRIBUTE
#define cindel delete
#define cindelarr delete[]
#define CIN_DUMP_ALLOCATIONS void
#endif
}