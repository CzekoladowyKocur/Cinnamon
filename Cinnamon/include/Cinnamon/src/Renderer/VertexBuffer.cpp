#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"

namespace Cinnamon {
	VertexBuffer::VertexBuffer(
		const STL::Unique<VulkanAllocator>& allocator,
		const VkDeviceSize reservedSize,
		const VertexBufferLayout& layout) noexcept
		:
		m_Allocator(allocator),
		m_Handle(VK_NULL_HANDLE),
		m_DeviceAllocation(),
		m_Layout(layout)
	{
		const VkBufferCreateInfo vertexBufferCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.size{ reservedSize },
			.usage{ VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT },
			.sharingMode{ VK_SHARING_MODE_EXCLUSIVE },
			.queueFamilyIndexCount{ VK_QUEUE_FAMILY_IGNORED },
			.pQueueFamilyIndices{ nullptr },
		};

		CIN_VERIFY(m_Allocator->AllocateBuffer(vertexBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, m_DeviceAllocation, m_Handle));
	}

	VertexBuffer::~VertexBuffer() noexcept
	{
		m_Allocator->DestroyBuffer(m_Handle, m_DeviceAllocation);
	}

	VkBuffer VertexBuffer::GetHandle() const
	{
		return m_Handle;
	}

	const VertexBufferLayout& VertexBuffer::GetLayout() const
	{
		return m_Layout;
	}

	void VertexBuffer::SetData(const void* data, const VkDeviceSize size, const VkDeviceSize offset)
	{
		Byte* const destination{ reinterpret_cast<Byte*>(m_Allocator->MapMemory(m_DeviceAllocation)) };
		memcpy(destination + offset, data, size);
		m_Allocator->UnmapMemory(m_DeviceAllocation);
	}
}