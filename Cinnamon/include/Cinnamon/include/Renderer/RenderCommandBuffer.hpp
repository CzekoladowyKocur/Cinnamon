#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	class Device;
	class Swapchain;

	class RenderCommandBuffer final
	{
	private:
		NON_COPYABLE(RenderCommandBuffer)
	public:
		explicit RenderCommandBuffer(
			const STL::Unique<Device>& device, 
			const uint32_t commandBufferCount) noexcept;

		explicit RenderCommandBuffer(
			const STL::Unique<Device>& device, 
			const STL::Unique<Swapchain>& swapchain) noexcept;
		
		~RenderCommandBuffer() noexcept;

		void Begin(const uint32_t frameIndex);
		void End(const uint32_t frameIndex);
		void Submit(const uint32_t frameIndex);
		void Wait(const uint32_t frameIndex);

		[[nodiscard]] VkCommandBuffer 
			GetCommandBuffer(const uint32_t frameIndex);

		[[nodiscard]] const VkCommandBufferInheritanceInfo& 
			GetCommandBufferInheritanceInfo() const;
	private:
		const STL::Unique<Device>& m_Device;
		Swapchain* m_Swapchain;

		VkCommandPool m_CommandPool;
		VkCommandBufferInheritanceInfo m_CommandBufferInheritanceInfo;

		STL::Vector<VkCommandBuffer> m_PrimaryCommandBuffers;
		STL::Vector<VkFence> m_WaitFences;
	};
}