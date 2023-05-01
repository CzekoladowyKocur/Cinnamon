#include "Cinnamon/include/Renderer/Device.h"
#include "Cinnamon/include/Renderer/Surface.h"
/* For physical device */
#include "Cinnamon/include/Renderer/GraphicsContext.h"

namespace Cinnamon {
	Device::Device(const STL::Unique<Surface>& surface)
		:
		m_PhysicalDevice(GraphicsContext::GetPhysicalDevice()),
		m_LogicalDevice(VK_NULL_HANDLE),
		m_QueueFamilies(),
		m_Queues(),
		m_GraphicsCommandPool(VK_NULL_HANDLE),
		m_ComputeCommandPool(VK_NULL_HANDLE),
		m_TransferCommandPool(VK_NULL_HANDLE),
		m_PresentCommandPool(VK_NULL_HANDLE)
	{
		/* Surface is created before picking any queue families to select a dedicated present queue */
		uint32_t queueFamilyPropertiesCount{ 0U };
		vkGetPhysicalDeviceQueueFamilyProperties(
			m_PhysicalDevice,
			&queueFamilyPropertiesCount,
			nullptr);

		if (queueFamilyPropertiesCount == 0U)
		{
			CIN_CRITICAL("Failed to retrieve queue family properties");
			CIN_PANIC_EXIT();
		}

		STL::Vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			m_PhysicalDevice,
			&queueFamilyPropertiesCount,
			&queueFamilyProperties[0U]);

		CIN_TRACE("Vulkan queue families:");
		for (uint32_t queueFamilyIndex{ 0U }; queueFamilyIndex < static_cast<uint32_t>(queueFamilyProperties.size()); ++queueFamilyIndex)
		{
			const uint32_t queueCount{ queueFamilyProperties[queueFamilyIndex].queueCount };
			const uint32_t queueFlags{ queueFamilyProperties[queueFamilyIndex].queueFlags };

			if (queueCount > 0U)
			{
				VkBool32 presentationSupported{ VK_FALSE };
				VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
					m_PhysicalDevice,
					queueFamilyIndex,
					surface->GetHandle(),
					&presentationSupported));

				if (presentationSupported)
				{
					if (m_QueueFamilies.Present == QueueFamilies::Absent)
					{
						CIN_TRACE("--Found present queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
						m_QueueFamilies.Present = static_cast<int32_t>(queueFamilyIndex);
						m_QueueFamilies.PresentQueueCount = queueCount;
					}
				}

				if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					CIN_TRACE("--Found graphics queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
					m_QueueFamilies.Graphics = static_cast<int32_t>(queueFamilyIndex);
					m_QueueFamilies.GraphicsQueueCount = queueCount;
					continue;
				}

				if (queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					CIN_TRACE("--Found compute queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
					m_QueueFamilies.Compute = static_cast<int32_t>(queueFamilyIndex);
					m_QueueFamilies.ComputeQueueCount = queueCount;

					if (presentationSupported)
					{
						CIN_TRACE("--Found present queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
						m_QueueFamilies.Present = static_cast<int32_t>(queueFamilyIndex);
						m_QueueFamilies.PresentQueueCount = queueCount;
					}
					continue;
				}

				/* Used for fast memory copying operations */
				if (queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					CIN_TRACE("--Found transfer queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
					m_QueueFamilies.Transfer = static_cast<int32_t>(queueFamilyIndex);
					m_QueueFamilies.TransferQueueCount = queueCount;
					continue;
				}
			}
		}

		if (m_QueueFamilies.Present == QueueFamilies::Absent)
		{
			CIN_CRITICAL("Failed to find a suitable present family");
			CIN_PANIC_EXIT();
		}

		if (m_QueueFamilies.Graphics == QueueFamilies::Absent)
		{
			CIN_CRITICAL("Failed to find a suitable graphics family");
			CIN_PANIC_EXIT();
		}

		constexpr float defaultQueuePriority{ 1.0f };
		STL::Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
		if (PresentAndGraphicsFamiliesShared())
		{
			CIN_ASSERT(m_QueueFamilies.Graphics == m_QueueFamilies.Present);
			CIN_ASSERT(m_QueueFamilies.GraphicsQueueCount == m_QueueFamilies.PresentQueueCount);
			constexpr STL::Array<float, 2U> defaultQueuePriorities{ defaultQueuePriority, defaultQueuePriority };

			VkDeviceQueueCreateInfo graphicsQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ m_QueueFamilies.Graphics },
				.queueCount{ m_QueueFamilies.GraphicsQueueCount > 1U ? 2U : 1U }, /* If queue family is shared, attempt using a different queue if more than 1 is available */
				.pQueuePriorities{ m_QueueFamilies.GraphicsQueueCount > 1U ? defaultQueuePriorities.data() : &defaultQueuePriority },
			};

			deviceQueueCreateInfos.emplace_back(std::move(graphicsQueueCreateInfo));
		}
		else
		{
			VkDeviceQueueCreateInfo presentQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ static_cast<uint32_t>(m_QueueFamilies.Present) },
				.queueCount{ 1U },
				.pQueuePriorities{ &defaultQueuePriority },
			};

			deviceQueueCreateInfos.emplace_back(std::move(presentQueueCreateInfo));
			VkDeviceQueueCreateInfo graphicsQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ m_QueueFamilies.Graphics },
				.queueCount{ 1U },
				.pQueuePriorities{ &defaultQueuePriority },
			};

			deviceQueueCreateInfos.emplace_back(std::move(graphicsQueueCreateInfo));
		}

		/* Transfer queue */
		if (m_QueueFamilies.Transfer != QueueFamilies::Absent)
		{
			VkDeviceQueueCreateInfo transferQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ m_QueueFamilies.Transfer },
				.queueCount{ 1U },
				.pQueuePriorities{ &defaultQueuePriority },
			};

			deviceQueueCreateInfos.emplace_back(std::move(transferQueueCreateInfo));
		}

		/* None for now */
		constexpr VkPhysicalDeviceFeatures enabledFeatures{};

		auto requestedLayers{ Platform::GetRequestedVulkanDeviceLayers() };
		auto requiredExtensions{ Platform::GetRequiredVulkanDeviceExtensions() };

		/* Check device layer support */
		if (!requestedLayers.empty())
		{
			uint32_t availableLayerCount{ 0U };
			VK_CHECK(vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &availableLayerCount, nullptr));
			if (availableLayerCount != 0U)
			{
				STL::Vector<VkLayerProperties> availableLayers(availableLayerCount);
				VK_CHECK(vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &availableLayerCount, &availableLayers[0U]));

				/* Continues program even if requested layer isn't supported */
				for (uint32_t i{ 0U }; i < requestedLayers.size(); ++i)
				{
					bool found{ false };
					CIN_TRACE("Requested layer: {}", requestedLayers[i]);
					for (uint32_t j{ 0U }; j < availableLayers.size(); ++j)
						if (strcmp(requestedLayers[i], availableLayers[j].layerName) == 0U)
						{
							found = true;
							CIN_TRACE("Found layer: {}", requestedLayers[i]);
							break;
						}

					/* If not found, remove the requested layer */
					if (!found)
					{
						auto iter{ std::find(requestedLayers.begin(), requestedLayers.end(), requestedLayers[i]) };
						CIN_ASSERT(iter != requestedLayers.end());
						requestedLayers.erase(iter);

						CIN_WARN("Failed to find requested layer: {}", requestedLayers[i]);
					}
				}
			}
		}

		/* Check device extension support */
		if (!requiredExtensions.empty())
		{
			uint32_t availableExtensionCount{ 0U };
			VK_CHECK(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &availableExtensionCount, nullptr));

			if (availableExtensionCount == 0U && !requiredExtensions.empty())
			{
				CIN_CRITICAL("Requested vulkan extensions, but none are available");
				CIN_PANIC_EXIT();
			}

			STL::Vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
			VK_CHECK(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &availableExtensionCount, &availableExtensions[0]));

			for (uint32_t i{ 0U }; i < requiredExtensions.size(); ++i)
			{
				bool found{ false };
				CIN_TRACE("Requested extension: {}", requiredExtensions[i]);
				for (uint32_t j{ 0U }; j < availableExtensions.size(); ++j)
					if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0U)
					{
						CIN_TRACE("Found extension: {}", requiredExtensions[i]);
						found = true;
						break;
					}

				if (!found)
				{
					CIN_CRITICAL("Failed to find extension with name: {}", requiredExtensions[i]);
					CIN_PANIC_EXIT();
				}
			}
		}

		CIN_ASSERT(!deviceQueueCreateInfos.empty(), "No device queues requested");
		const VkDeviceCreateInfo deviceCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.queueCreateInfoCount{ static_cast<uint32_t>(deviceQueueCreateInfos.size()) },
			.pQueueCreateInfos{ &deviceQueueCreateInfos[0U] },
			.enabledLayerCount{ requestedLayers.empty() ? 0U : static_cast<uint32_t>(requestedLayers.size()) },
			.ppEnabledLayerNames{ requestedLayers.empty() ? nullptr : &requestedLayers[0U] },
			.enabledExtensionCount{ requiredExtensions.empty() ? 0U : static_cast<uint32_t>(requiredExtensions.size()) },
			.ppEnabledExtensionNames{  requiredExtensions.empty() ? nullptr : &requiredExtensions[0U] },
			.pEnabledFeatures{ &enabledFeatures },
		};

		VK_CHECK(vkCreateDevice(
			m_PhysicalDevice,
			&deviceCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_LogicalDevice));

		if (PresentAndGraphicsFamiliesShared())
		{
			CIN_ASSERT(m_QueueFamilies.Graphics == m_QueueFamilies.Present);
			if (PresentAndGraphicsQueuesCanBeSeparate())
			{
				CIN_ASSERT(m_QueueFamilies.GraphicsQueueCount > 1U);
				vkGetDeviceQueue(
					m_LogicalDevice,
					m_QueueFamilies.Graphics,
					0U, /* Pick first queue in a queue family */
					&m_Queues.Graphics);

				vkGetDeviceQueue(
					m_LogicalDevice,
					m_QueueFamilies.Present,
					1U, /* Pick second queue in a queue family */
					&m_Queues.Present);
			}
			else
			{
				vkGetDeviceQueue(
					m_LogicalDevice,
					m_QueueFamilies.Graphics,
					0U, /* Pick first queue in a queue family */
					&m_Queues.Graphics);

				m_Queues.Present = m_Queues.Graphics;
			}
		}
		else
		{
			vkGetDeviceQueue(
				m_LogicalDevice,
				m_QueueFamilies.Graphics,
				0U,
				&m_Queues.Graphics);

			vkGetDeviceQueue(
				m_LogicalDevice,
				m_QueueFamilies.Present,
				0U,
				&m_Queues.Present);
		}

		/* Pick a transfer queue if available, if not - default to graphics queue */
		if (m_QueueFamilies.Transfer != QueueFamilies::Absent)
			vkGetDeviceQueue(
				m_LogicalDevice,
				m_QueueFamilies.Transfer,
				0U, /* Pick first queue in a queue family */
				&m_Queues.Transfer);
		else
		{
			CIN_WARN("Transfer queue family was not found. Defaulting transfer queue to a graphics queue");
			m_Queues.Transfer = m_Queues.Graphics;
			m_QueueFamilies.Transfer = m_QueueFamilies.Graphics;
		}

		CIN_ASSERT(m_Queues.Graphics != VK_NULL_HANDLE, "Graphics queue is invalid");
		CIN_ASSERT(m_Queues.Present != VK_NULL_HANDLE, "Present is invalid");
		CIN_ASSERT(m_Queues.Transfer != VK_NULL_HANDLE, "Transfer queue is invalid");

		/* Create graphics command pool */
		{
			const VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ m_QueueFamilies.Graphics },
			};

			VK_CHECK(vkCreateCommandPool(
				m_LogicalDevice,
				&graphicsCommandPoolCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_GraphicsCommandPool));
		}

		CIN_TRACE("Created graphics command pool from queue family {}", m_QueueFamilies.Graphics);
		/* Create a transfer command pool if transfer queue family is available, if not - default to graphics command pool */
		if (m_QueueFamilies.Transfer != m_QueueFamilies.Graphics)
		{
			/* Create transfer command pool */
			const VkCommandPoolCreateInfo transferCommandPoolCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ m_QueueFamilies.Transfer },
			};

			VK_CHECK(vkCreateCommandPool(
				m_LogicalDevice,
				&transferCommandPoolCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_TransferCommandPool));

			CIN_TRACE("Created transfer command pool from queue family {}", m_QueueFamilies.Transfer);
		}
		else
			m_TransferCommandPool = m_GraphicsCommandPool;
	}

	Device::~Device()
	{
		[[likely]]
		if (m_LogicalDevice)
		{
			VK_CHECK(vkDeviceWaitIdle(
				m_LogicalDevice));

			vkDestroyCommandPool(
				m_LogicalDevice,
				m_GraphicsCommandPool,
				GraphicsContext::GetAllocator());

			vkDestroyCommandPool(
				m_LogicalDevice,
				m_TransferCommandPool,
				GraphicsContext::GetAllocator());

			vkDestroyDevice(
				m_LogicalDevice,
				GraphicsContext::GetAllocator());
		}
	}

	void Device::PerformSingleSubmitGraphicsOperation(const std::function<void(VkCommandBuffer)> operation)
	{
#if 1
		CIN_ASSERT(m_GraphicsCommandPool, "Invalid graphics command pool");
		const VkCommandBufferAllocateInfo commandBufferAllocateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
			.pNext{ nullptr },
			.commandPool{ m_GraphicsCommandPool },
			.level{ VK_COMMAND_BUFFER_LEVEL_PRIMARY },
			.commandBufferCount{ 1U },
		};

		VkCommandBuffer commandBuffer;
		VK_CHECK(vkAllocateCommandBuffers(
			m_LogicalDevice,
			&commandBufferAllocateInfo,
			&commandBuffer));

		const VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
			.pNext{ nullptr },
			.flags{ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT },
			.pInheritanceInfo{ nullptr },
		};

		VK_CHECK(vkBeginCommandBuffer(
			commandBuffer,
			&commandBufferBeginInfo));

		/* User code */
		operation(commandBuffer);

		VK_CHECK(vkEndCommandBuffer(
			commandBuffer));

		const VkSubmitInfo submitInfo
		{
			.sType{ VK_STRUCTURE_TYPE_SUBMIT_INFO },
			.pNext{ nullptr },
			.waitSemaphoreCount{ 0U },
			.pWaitSemaphores{ nullptr },
			.pWaitDstStageMask{ 0U /* VK_PIPELINE_STAGE_GRAPHICS_BIT */},
			.commandBufferCount{ 1U },
			.pCommandBuffers{ &commandBuffer },
			.signalSemaphoreCount{ 0U },
			.pSignalSemaphores{ nullptr },
		};

		constexpr VkFenceCreateInfo fenceCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
		};

		VkFence executionHasFinishedFence;
		VK_CHECK(vkCreateFence(
			m_LogicalDevice,
			&fenceCreateInfo,
			GraphicsContext::GetAllocator(),
			&executionHasFinishedFence));

		VK_CHECK(vkQueueSubmit(
			m_Queues.Graphics,
			1U,
			&submitInfo,
			executionHasFinishedFence));

		VK_CHECK(vkWaitForFences(
			m_LogicalDevice,
			1U,
			&executionHasFinishedFence,
			VK_FALSE,
			std::numeric_limits<uint64_t>::max()));

		vkFreeCommandBuffers(
			m_LogicalDevice,
			m_GraphicsCommandPool,
			1U,
			&commandBuffer);

		vkDestroyFence(
			m_LogicalDevice,
			executionHasFinishedFence,
			GraphicsContext::GetAllocator());
#endif
	}

	VkPhysicalDevice Device::GetPhysicalDevice()
	{
		return m_PhysicalDevice;
	}

	VkDevice Device::GetLogicalDevice()
	{
		return m_LogicalDevice;
	}

	const QueueFamilies& Device::GetQueueFamilies() const
	{
		return m_QueueFamilies;
	}

	const Queues& Device::GetQueues() const
	{
		return m_Queues;
	}

	bool Device::PresentAndGraphicsFamiliesShared()
	{
		return m_QueueFamilies.Graphics == m_QueueFamilies.Present;
	}

	bool Device::PresentAndGraphicsQueuesCanBeSeparate()
	{
		return
			m_QueueFamilies.Graphics != m_QueueFamilies.Present ||
			m_QueueFamilies.GraphicsQueueCount > 2;
	}
}