#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"
#include "VulkanMemoryAllocator/vk_mem_alloc.h"

namespace Cinnamon {
	class Device;

	class VulkanAllocator final
	{
	private:
		NON_COPYABLE(VulkanAllocator)
	public:
		explicit VulkanAllocator(const STL::Unique<Device>& device) noexcept;		
		~VulkanAllocator() noexcept;

		const STL::Unique<Device>& GetDevice() const;

		Errr AllocateBuffer(
			const VkBufferCreateInfo& bufferCreateInfo,
			const VmaMemoryUsage memoryUsage,
			VmaAllocation& outAllocation,
			VkBuffer& outBuffer);

		void DestroyBuffer(
			const VkBuffer buffer,
			const VmaAllocation allocation);

		Errr AllocateImage(
			const VkImageCreateInfo& imageCreateInfo,
			const VmaMemoryUsage memoryUsage,
			VmaAllocation& outAllocation,
			VkImage& outImage);

		void DestroyImage(
			const VkImage image,
			const VmaAllocation allocation);

		[[nodiscard]] void* MapMemory(const VmaAllocation allocation);
		void UnmapMemory(const VmaAllocation allocation);
	private:
		const STL::Unique<Device>& m_Device;
		
		VmaAllocator m_Allocator;
		VkDeviceSize m_TotalBytesAllocated;
	};
}