#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"
#include "Cinnamon/include/Renderer/Queue.hpp"
#include "Cinnamon/include/Core/Window.hpp"

namespace Cinnamon {
	class Window;
	class Swapchain;
	class VulkanAllocator;

	class Device final
	{
	private:
		NON_COPYABLE(Device)
	public:
		explicit Device(const STL::Unique<Window>& window) noexcept;
		~Device() noexcept;

		void PerformSingleSubmitGraphicsOperation(const std::function<void(VkCommandBuffer)> operation);
		
		VkPhysicalDevice GetPhysicalDevice();
		VkDevice GetLogicalDevice();

		const QueueFamilies& GetQueueFamilies() const;
		const Queues& GetQueues() const;
	
		bool PresentAndGraphicsFamiliesShared();
		bool PresentAndGraphicsQueuesCanBeSeparate();
	private:
		/* Physical device connection */
		VkPhysicalDevice	m_PhysicalDevice;
		VkDevice			m_LogicalDevice;
		/* Queue families */
		QueueFamilies m_QueueFamilies;
		Queues m_Queues;

		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_ComputeCommandPool;
		VkCommandPool m_TransferCommandPool;
		VkCommandPool m_PresentCommandPool;
	};
}