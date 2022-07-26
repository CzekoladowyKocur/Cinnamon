#pragma once
#include "Cinnamon/include/Core/Core.h"

namespace Cinnamon {
#if 0
	template<typename T>
	class LinearAllocator 
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
	public:
		template<typename U>
		struct rebind 
		{
			typedef LinearAllocator<U> other;
		};
	public:
		explicit LinearAllocator(T* begin, const std::size_t size) noexcept
			:
			m_Begin(nullptr),
			m_End(nullptr),
			m_Current(nullptr)
		{
			CIN_UNUSED(begin);
			CIN_UNUSED(size);
			CIN_UNIMPLEMENTED();
		}

		explicit LinearAllocator(const std::size_t objectCount)
			:
			m_Begin(nullptr),
			m_End(nullptr),
			m_Current(nullptr)
		{
			CIN_UNUSED(objectCount);
			CIN_UNIMPLEMENTED();
		}
		
		inline explicit LinearAllocator(const LinearAllocator& other)
			:
			m_Begin(nullptr),
			m_End(nullptr),
			m_Current(nullptr)
		{
			CIN_UNUSED(other);
			CIN_UNIMPLEMENTED();
		}
		
		template<typename U>
		inline explicit LinearAllocator(const LinearAllocator<U>& other)
			:
			m_Begin(nullptr),
			m_End(nullptr),
			m_Current(nullptr)
		{
			CIN_UNUSED(other);
			CIN_UNIMPLEMENTED();
		}

		inline pointer address(reference ref)
		{ 
			return &ref;
		}

		inline [[nodiscard]] const_pointer address(const_reference ref)
		{ 
			return &ref;
		}

		inline [[nodiscard]] pointer allocate(size_type count)
		{
			CIN_UNUSED(count);
			return nullptr;
		}

		inline void deallocate(pointer ptr, size_type)
		{
			delete ptr;
		}

		inline size_type max_size() const 
		{
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		inline void construct(pointer ptr, const T& t) 
		{ 
			new(ptr)
			T(t); 
		}

		inline void destroy(pointer ptr)
		{ 
			ptr->~T();
		}

		inline bool operator==(const LinearAllocator& other)
		{ 
			CIN_UNIMPLEMENTED();
			CIN_UNUSED(other);
			return true; 
		}
		
		inline bool operator!=(const LinearAllocator& other)
		{ 
			return !(*this == other);
		}
	public:
		T* m_Begin;
		T* m_End;
		T* m_Current;
	};
#endif
}