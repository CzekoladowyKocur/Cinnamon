#include "Cinnamon/include/Renderer/UniformBuffer.hpp"

namespace Cinnamon {
	UniformBuffer::UniformBuffer(
		const STL::Unique<VulkanAllocator>& allocator,
		const size_t size, 
		const uint32_t binding) noexcept
		:
		m_Allocator(allocator),
		m_BindingSlot(binding),
		m_Handle(VK_NULL_HANDLE),
		m_DeviceAllocation(),
		m_DescriptorBufferInfo()
	{
		const VkBufferCreateInfo uniformBufferCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.size{ size },
			.usage{ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT },
			.sharingMode{ VK_SHARING_MODE_EXCLUSIVE },
			.queueFamilyIndexCount{ 0U },
			.pQueueFamilyIndices{ nullptr }
		};

		CIN_VERIFY(m_Allocator->AllocateBuffer(
			uniformBufferCreateInfo, 
			VMA_MEMORY_USAGE_CPU_ONLY, 
			m_DeviceAllocation, 
			m_Handle));
			
		m_DescriptorBufferInfo.buffer = m_Handle;
		m_DescriptorBufferInfo.offset = 0U;
		m_DescriptorBufferInfo.range = size;
	}

	UniformBuffer::~UniformBuffer() noexcept
	{
		m_Allocator->DestroyBuffer(
			m_Handle,
			m_DeviceAllocation);
	}

	void UniformBuffer::SetData(
		const void* const data,
		const uint64_t size,
		const uint64_t offset)
	{
		void* const destination{ m_Allocator->MapMemory(m_DeviceAllocation) };
		memcpy(reinterpret_cast<std::byte*>(destination) + offset, data, size);
		m_Allocator->UnmapMemory(m_DeviceAllocation);
	}

	Byte* UniformBuffer::MapData()
	{
		return reinterpret_cast<Byte*>(m_Allocator->MapMemory(m_DeviceAllocation));
	}

	void UniformBuffer::Unmapdata()
	{
		m_Allocator->UnmapMemory(m_DeviceAllocation);
	}

	uint32_t UniformBuffer::GetBindingSlot() const
	{
		return m_BindingSlot;
	}

	const VkDescriptorBufferInfo&
		UniformBuffer::GetDescriptorBufferInfo() const
	{
		return m_DescriptorBufferInfo;
	}
}