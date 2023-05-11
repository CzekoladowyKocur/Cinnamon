#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"

namespace Cinnamon {
	RenderCommandBuffer::RenderCommandBuffer(
		const STL::Unique<Device>& device, 
		const uint32_t commandBufferCount) noexcept
		:
		m_Device(device),
		m_Swapchain(nullptr),
		m_CommandPool(VK_NULL_HANDLE),
		m_CommandBufferInheritanceInfo(),
		m_PrimaryCommandBuffers(),
		m_WaitFences()
	{
		m_PrimaryCommandBuffers.resize(commandBufferCount);
		m_WaitFences.resize(commandBufferCount);

		const VkCommandPoolCreateInfo commandPoolCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT },
			.queueFamilyIndex{ m_Device->GetQueueFamilies().Graphics }
		};

		VK_CHECK(vkCreateCommandPool(
			m_Device->GetLogicalDevice(),
			&commandPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_CommandPool));

		const VkCommandBufferAllocateInfo commandBufferAllocateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
			.pNext{ nullptr },
			.commandPool{ m_CommandPool },
			.level{ VK_COMMAND_BUFFER_LEVEL_PRIMARY },
			.commandBufferCount{ commandBufferCount }
		};

		VK_CHECK(vkAllocateCommandBuffers(
			m_Device->GetLogicalDevice(),
			&commandBufferAllocateInfo,
			m_PrimaryCommandBuffers.data()));
		
		const VkFenceCreateInfo fenceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		for (uint32_t i{ 0U }; i < commandBufferCount; ++i)
			VK_CHECK(vkCreateFence(
				m_Device->GetLogicalDevice(),
				&fenceCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_WaitFences[i]));
	}

	RenderCommandBuffer::RenderCommandBuffer(
		const STL::Unique<Device>& device, 
		const STL::Unique<Swapchain>& swapchain) noexcept
		:
		m_Device(device),
		m_Swapchain(swapchain.get()),
		m_CommandPool(VK_NULL_HANDLE),
		m_CommandBufferInheritanceInfo(),
		m_PrimaryCommandBuffers(),
		m_WaitFences()
	{
		const uint32_t swapchainImageCount{ swapchain->GetImageCount() };

		m_PrimaryCommandBuffers.resize(swapchainImageCount);
		for (uint32_t i{ 0U }; i < swapchainImageCount; ++i)
			m_PrimaryCommandBuffers[i] = swapchain->GetCommandBuffer(i);

		const VkFenceCreateInfo fenceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		m_WaitFences.resize(swapchainImageCount);
		for (uint32_t i{ 0U }; i < swapchainImageCount; ++i)
			VK_CHECK(vkCreateFence(
				m_Device->GetLogicalDevice(),
				&fenceCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_WaitFences[i]));
	}

	RenderCommandBuffer::~RenderCommandBuffer() noexcept
	{
		if (!m_WaitFences.empty())
		{
			VK_CHECK(vkWaitForFences(
				m_Device->GetLogicalDevice(),
				static_cast<uint32_t>(m_WaitFences.size()),
				m_WaitFences.data(),
				VK_TRUE,
				std::numeric_limits<std::uint64_t>::max()));
		}

		for (size_t i{ 0U }; i < m_WaitFences.size(); ++i)
		{
			vkDestroyFence(
				m_Device->GetLogicalDevice(),
				m_WaitFences[i],
				GraphicsContext::GetAllocator());
		}
		
		if (m_Swapchain)
			return;

		vkDestroyCommandPool(
			m_Device->GetLogicalDevice(),
			m_CommandPool,
			GraphicsContext::GetAllocator());
	}

	void RenderCommandBuffer::Begin(const uint32_t frameIndex)
	{
		if (m_Swapchain)
		{
			m_WaitFences[frameIndex] = m_Swapchain->GetWaitFence(frameIndex);
			m_PrimaryCommandBuffers[frameIndex] = m_Swapchain->GetCommandBuffer(frameIndex);
		}

		const VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
			.pNext{ nullptr },
			.flags{ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT },
			.pInheritanceInfo{ nullptr }
		};
		
		VK_CHECK(vkBeginCommandBuffer(
			m_PrimaryCommandBuffers[frameIndex],
			&commandBufferBeginInfo));
	}

	void RenderCommandBuffer::End(const uint32_t frameIndex)
	{
		VK_CHECK(vkEndCommandBuffer(m_PrimaryCommandBuffers[frameIndex]));
	}

	void RenderCommandBuffer::Submit(const uint32_t frameIndex)
	{
		if (m_Swapchain)
			return;

		const VkSubmitInfo submitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0U,
			.pWaitSemaphores = nullptr,
			.commandBufferCount = 1U,
			.pCommandBuffers = &m_PrimaryCommandBuffers[frameIndex],
			.signalSemaphoreCount = 0U,
			.pSignalSemaphores = nullptr,
		};

		VK_CHECK(vkWaitForFences(
			m_Device->GetLogicalDevice(),
			1,
			&m_WaitFences[frameIndex],
			VK_TRUE,
			std::numeric_limits<std::uint64_t>::max()));

		VK_CHECK(vkResetFences(
			m_Device->GetLogicalDevice(),
			1,
			&m_WaitFences[frameIndex]));

		VK_CHECK(vkQueueSubmit(
			m_Device->GetQueues().Graphics,
			1,
			&submitInfo,
			m_WaitFences[frameIndex]));
	}

	void RenderCommandBuffer::Wait(const uint32_t frameIndex)
	{
		if (m_Swapchain)
			return;

		VK_CHECK(vkWaitForFences(
			m_Device->GetLogicalDevice(),
			1,
			&m_WaitFences[frameIndex],
			VK_TRUE,
			std::numeric_limits<std::uint64_t>::max()));
	}

	VkCommandBuffer RenderCommandBuffer::GetCommandBuffer(const uint32_t frameIndex)
	{
		return m_PrimaryCommandBuffers[frameIndex];
	}

	const VkCommandBufferInheritanceInfo& RenderCommandBuffer::GetCommandBufferInheritanceInfo() const
	{
		return m_CommandBufferInheritanceInfo;
	}
}