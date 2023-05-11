#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"

namespace Cinnamon {
	VulkanAllocator::VulkanAllocator(const STL::Unique<Device>& device) noexcept
		:
		m_Device(device),
		m_Allocator(),
		m_TotalBytesAllocated(0U)
	{
		FunctionVariable const VmaVulkanFunctions f_VulkanFunctions
		{
			.vkGetInstanceProcAddr{ vkGetInstanceProcAddr },
			.vkGetDeviceProcAddr{ vkGetDeviceProcAddr },
			.vkGetPhysicalDeviceProperties{ vkGetPhysicalDeviceProperties },
			.vkGetPhysicalDeviceMemoryProperties{ vkGetPhysicalDeviceMemoryProperties },
			.vkAllocateMemory{ vkAllocateMemory },
			.vkFreeMemory{ vkFreeMemory },
			.vkMapMemory{ vkMapMemory },
			.vkUnmapMemory{ vkUnmapMemory },
			.vkFlushMappedMemoryRanges{ vkFlushMappedMemoryRanges },
			.vkInvalidateMappedMemoryRanges{ vkInvalidateMappedMemoryRanges },
			.vkBindBufferMemory{ vkBindBufferMemory },
			.vkBindImageMemory{ vkBindImageMemory },
			.vkGetBufferMemoryRequirements{ vkGetBufferMemoryRequirements },
			.vkGetImageMemoryRequirements{ vkGetImageMemoryRequirements },
			.vkCreateBuffer{ vkCreateBuffer },
			.vkDestroyBuffer{ vkDestroyBuffer },
			.vkCreateImage{ vkCreateImage },
			.vkDestroyImage{ vkDestroyImage },
			.vkCmdCopyBuffer{ vkCmdCopyBuffer },
			.vkGetBufferMemoryRequirements2KHR{ vkGetBufferMemoryRequirements2KHR },
			.vkGetImageMemoryRequirements2KHR{ vkGetImageMemoryRequirements2KHR },
			.vkBindBufferMemory2KHR{ vkBindBufferMemory2KHR },
			.vkBindImageMemory2KHR{ vkBindImageMemory2KHR },
			.vkGetPhysicalDeviceMemoryProperties2KHR{ vkGetPhysicalDeviceMemoryProperties2KHR },
			.vkGetDeviceBufferMemoryRequirements{ vkGetDeviceBufferMemoryRequirements },
			.vkGetDeviceImageMemoryRequirements{ vkGetDeviceImageMemoryRequirements },
		};

		const VmaAllocatorCreateInfo allocatorCreateInfo
		{
			.flags{ 0U },
			.physicalDevice{ m_Device->GetPhysicalDevice()},
			.device{ m_Device->GetLogicalDevice() },
			.preferredLargeHeapBlockSize{ 0U }, /* default */
			.pAllocationCallbacks{ GraphicsContext::GetAllocator() },
			.pDeviceMemoryCallbacks{ nullptr },
			.pHeapSizeLimit{ nullptr },
			.pVulkanFunctions{ &f_VulkanFunctions },
			.instance{ GraphicsContext::GetInstance() },
			.vulkanApiVersion{ VK_API_VERSION_1_3 },
			.pTypeExternalMemoryHandleTypes{ nullptr }
		};

		CIN_VERIFY(vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator) == VK_SUCCESS);
	}

	VulkanAllocator::~VulkanAllocator() noexcept
	{
		vmaDestroyAllocator(m_Allocator);
	}

	const STL::Unique<Device>& VulkanAllocator::GetDevice() const
	{
		return m_Device;
	}

	Errr VulkanAllocator::AllocateBuffer(
		const VkBufferCreateInfo& bufferCreateInfo,
		const VmaMemoryUsage memoryUsage,
		VmaAllocation& outAllocation,
		VkBuffer& outBuffer)
	{
		const VmaAllocationCreateInfo bufferAllocationCreateInfo
		{
			.flags{ 0U },
			.usage{ memoryUsage },
			.requiredFlags{ 0U },
			.preferredFlags{ 0U },
			.memoryTypeBits{ 0U },
			.pool{ VK_NULL_HANDLE },
			.pUserData{ nullptr },
			.priority{ 0.0f },
		};

		const VkResult result
		{
			vmaCreateBuffer
			(
				m_Allocator,
				&bufferCreateInfo,
				&bufferAllocationCreateInfo,
				&outBuffer,
				&outAllocation,
				nullptr
			)
		};

		[[unlikely]]
		if (result != VK_SUCCESS)
			return Error::Failure;

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(
			m_Allocator,
			outAllocation,
			&allocationInfo);

		m_TotalBytesAllocated += allocationInfo.size;
		return Error::Success;
	}

	void VulkanAllocator::DestroyBuffer(
		const VkBuffer buffer,
		const VmaAllocation allocation)
	{
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(
			m_Allocator,
			allocation,
			&allocationInfo);

		m_TotalBytesAllocated -= allocationInfo.size;
		vmaDestroyBuffer(
			m_Allocator,
			buffer,
			allocation);
	}

	Errr VulkanAllocator::AllocateImage(
		const VkImageCreateInfo& imageCreateInfo, 
		const VmaMemoryUsage memoryUsage, 
		VmaAllocation& outAllocation, 
		VkImage& outImage)
	{
		const VmaAllocationCreateInfo imageAllocationCreateInfo
		{
			.flags{ 0U },
			.usage{ memoryUsage },
			.requiredFlags{ 0U },
			.preferredFlags{ 0U },
			.memoryTypeBits{ 0U },
			.pool{ VK_NULL_HANDLE },
			.pUserData{ nullptr },
			.priority{ 0.0f },
		};

		const VkResult result
		{
			vmaCreateImage
			(
				m_Allocator,
				&imageCreateInfo,
				&imageAllocationCreateInfo,
				&outImage,
				&outAllocation,
				nullptr
			)
		};

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(
			m_Allocator,
			outAllocation,
			&allocationInfo);

		m_TotalBytesAllocated += allocationInfo.size;

		[[unlikely]]
		if (result != VK_SUCCESS)
			return Error::Failure;

		return Error::Success;
	}

	void VulkanAllocator::DestroyImage(
		const VkImage image, 
		const VmaAllocation allocation)
	{
		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(
			m_Allocator,
			allocation,
			&allocationInfo);

		m_TotalBytesAllocated -= allocationInfo.size;
		vmaDestroyImage(
			m_Allocator,
			image,
			allocation);
	}

	void* VulkanAllocator::MapMemory(const VmaAllocation allocation)
	{
		void* mappedMemory{ nullptr };
		vmaMapMemory(m_Allocator, allocation, &mappedMemory);
		return mappedMemory;
	}

	void VulkanAllocator::UnmapMemory(const VmaAllocation allocation)
	{
		vmaUnmapMemory(m_Allocator, allocation);
	}
}