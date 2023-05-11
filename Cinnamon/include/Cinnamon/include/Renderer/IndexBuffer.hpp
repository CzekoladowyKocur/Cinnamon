#pragma once
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"

namespace Cinnamon {
	class IndexBuffer final
	{
	private:
		NON_COPYABLE(IndexBuffer)
	public:
		explicit IndexBuffer(const STL::Unique<VulkanAllocator>& allocator, const size_t reservedSize) noexcept;
		~IndexBuffer() noexcept;

		[[nodiscard]] VkBuffer
			GetHandle() const;

		void SetData(const void* data, const VkDeviceSize size, const VkDeviceSize offset = 0U);
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;

		VkBuffer m_Handle;
		VmaAllocation m_DeviceAllocation;
	};
}