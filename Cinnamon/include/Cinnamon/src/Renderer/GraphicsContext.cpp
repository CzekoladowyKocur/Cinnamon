#include "Cinnamon/include/Renderer/GraphicsContext.h"
#include "Cinnamon/include/Renderer/Surface.h"
#include "Cinnamon/include/Renderer/Swapchain.h"
#include "Cinnamon/include/Core/Window.h"
#include "Platform/Platform.h"

namespace Cinnamon {
	InternalScope const Window* s_WindowContext{ nullptr };
	/* TODO: Change to GPU score system */
	InternalScope bool PhysicalDeviceMeetsRequirements(const VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		return physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	}
#if CIN_DEBUG
	InternalScope VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtilitiesCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		CIN_UNUSED(messageTypes);
		CIN_UNUSED(pUserData);

		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			{
				CIN_TRACE("{}", pCallbackData->pMessage);
				break;
			}

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			{
				CIN_INFO("{}", pCallbackData->pMessage);
				break;
			}

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			{
				CIN_WARN("{}", pCallbackData->pMessage);
				break;
			}

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			{
				CIN_ERROR("{}", pCallbackData->pMessage);
				break;
			}

			default:
			{
				CIN_ERROR("{}", pCallbackData->pMessage);
				break;
			}
		}

		return VK_FALSE;
	}
#endif
	bool GraphicsContext::Initialize()
	{
		VkResult success{ volkInitialize() };
		if (success != VK_SUCCESS)
		{
			CIN_CRITICAL("Failed to initialize volk");
			return false;
		}

		auto requestedLayers{ Platform::GetRequestedVulkanLayers() };
		auto requiredExtensions{ Platform::GetRequiredVulkanExtensions() };

		/* Check layer support */
		if (!requestedLayers.empty())
		{
			uint32_t availableLayerCount{ 0 };
			VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
			if (availableLayerCount != 0)
			{
				STL::Vector<VkLayerProperties> availableLayers(availableLayerCount);
				VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, &availableLayers[0]));

				/* Continues program even if requested layer isn't supported */
				for (uint32_t i{ 0 }; i < requestedLayers.size(); ++i)
				{
					bool found{ false };
					CIN_TRACE("Requested layer: {0}", requestedLayers[i]);
					for (uint32_t j{ 0 }; j < availableLayers.size(); ++j)
						if (strcmp(requestedLayers[i], availableLayers[j].layerName) == 0)
						{
							found = true;
							CIN_TRACE("Found layer: {0}", requestedLayers[i]);
							break;
						}

					/* If not found, remove the requested layer */
					if (!found)
					{
						auto iter{ std::find(requestedLayers.begin(), requestedLayers.end(), requestedLayers[i]) };
						CIN_ASSERT(iter != requestedLayers.end());
						requestedLayers.erase(iter);

						CIN_WARN("Failed to find requested layer: {0}", requestedLayers[i]);
					}
				}
			}
		}

		/* Check extension support */
		if (!requiredExtensions.empty())
		{
			uint32_t availableExtensionCount{ 0 };
			VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr));

			if (availableExtensionCount == 0 && !requiredExtensions.empty())
			{
				CIN_CRITICAL("Requested vulkan extensions, but none are available");
				return false;
			}

			STL::Vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
			VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, &availableExtensions[0]));

			for (uint32_t i{ 0 }; i < requiredExtensions.size(); ++i)
			{
				bool found{ false };
				CIN_TRACE("Requested extension: {0}", requiredExtensions[i]);
				for (uint32_t j{ 0 }; j < availableExtensions.size(); ++j)
					if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
					{
						CIN_TRACE("Found extension: {0}", requiredExtensions[i]);
						found = true;
						break;
					}

				if (!found)
				{
					CIN_CRITICAL("Failed to find extension with name: {0}", requiredExtensions[i]);
					return false;
				}
			}
		}

		constexpr VkApplicationInfo applicationInfo{
			.sType{ VK_STRUCTURE_TYPE_APPLICATION_INFO },
			.pNext{ nullptr },
			.pApplicationName{ "Application" },
			.applicationVersion{ VK_MAKE_VERSION(0, 0, 1) },
			.pEngineName{ "Cinnamon" },
			.engineVersion{ VK_MAKE_VERSION(0, 0, 1) },
			.apiVersion{ VK_MAKE_API_VERSION(0, 1, 3, 0) },
		};

		const VkInstanceCreateInfo instanceCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.pApplicationInfo{ &applicationInfo },
			.enabledLayerCount{ requestedLayers.empty() ? 0 : static_cast<uint32_t>(requestedLayers.size()) },
			.ppEnabledLayerNames{ requestedLayers.empty() ? nullptr : &requestedLayers[0] },
			.enabledExtensionCount{ requiredExtensions.empty() ? 0 : static_cast<uint32_t>(requiredExtensions.size()) },
			.ppEnabledExtensionNames{  requiredExtensions.empty() ? nullptr : &requiredExtensions[0] },
		};

		VK_CHECK(vkCreateInstance(
			&instanceCreateInfo,
			s_Allocator,
			&s_Instance));

		volkLoadInstance(s_Instance);
#ifdef CIN_DEBUG
		constexpr VkDebugUtilsMessengerCreateInfoEXT debugUtilitiesMessengerCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT },
			.pNext{ nullptr },
			.flags{ 0U },
			.messageSeverity{ 
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT 
			},
			.messageType{ 
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT},
			.pfnUserCallback{ VulkanDebugUtilitiesCallback },
			.pUserData{ nullptr },
		};

		const PFN_vkCreateDebugUtilsMessengerEXT _vkCreateDebugUtilsMessengerEXT{ reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
				s_Instance, 
				"vkCreateDebugUtilsMessengerEXT")) };

		CIN_ASSERT(_vkCreateDebugUtilsMessengerEXT);
		VK_CHECK(_vkCreateDebugUtilsMessengerEXT(
			s_Instance,
			&debugUtilitiesMessengerCreateInfo,
			s_Allocator,
			&s_DebugUtilitiesMessenger));
#endif
		uint32_t physicalDeviceCount{ 0 };
		VK_CHECK(vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, nullptr));
		STL::Vector<VkPhysicalDevice> availablePhysicalDevices(physicalDeviceCount);
		VK_CHECK(vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, &availablePhysicalDevices[0]));

		if (availablePhysicalDevices.empty())
		{
			CIN_CRITICAL("Failed to find a supported physical device");
			return false;
		}

		for (uint32_t i{ 0 }; i < availablePhysicalDevices.size(); ++i)
		{
			const VkPhysicalDevice physicalDevice{ availablePhysicalDevices[i] };

			if (PhysicalDeviceMeetsRequirements(physicalDevice))
			{
				s_PhysicalDevice = physicalDevice;
				break;
			}
		}

		/* If no discrete gpu was found, pick the first one */
		if (!s_PhysicalDevice)
			s_PhysicalDevice = availablePhysicalDevices.front();

		CIN_TRACE("Initialized graphics context");
		return true;
	}

	bool GraphicsContext::Shutdown()
	{
		VK_CHECK(vkDeviceWaitIdle(
			s_LogicalDevice));

		cindel s_Swapchain;
		cindel s_Surface;

		vkDestroyCommandPool(
			s_LogicalDevice,
			s_CommandPools.Graphics,
			s_Allocator);

		vkDestroyCommandPool(
			s_LogicalDevice,
			s_CommandPools.Transfer,
			s_Allocator);

		vkDestroyDevice(
			s_LogicalDevice,
			s_Allocator);
		
#ifdef CIN_DEBUG
		const PFN_vkDestroyDebugUtilsMessengerEXT _vkDestroyDebugUtilsMessengerEXT{ reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(s_Instance, "vkDestroyDebugUtilsMessengerEXT")) };
		CIN_ASSERT(_vkDestroyDebugUtilsMessengerEXT != nullptr);

		_vkDestroyDebugUtilsMessengerEXT(
			s_Instance,
			s_DebugUtilitiesMessenger,
			s_Allocator);
#endif
		vkDestroyInstance(
			s_Instance,
			s_Allocator);
#ifdef CIN_PLATFORM_WINDOWS
		const HMODULE sharedVulkanLibraryModule{ GetModuleHandleA("vulkan-1.dll") };
		CIN_ASSERT(sharedVulkanLibraryModule, "Invalid module");
		CIN_VERIFY(FreeLibrary(sharedVulkanLibraryModule));
#else
#endif
		return true;
	}

	bool GraphicsContext::CreateSurface(const Window* const windowContext)
	{
		/* Surface is created before picking any queue families to select a dedicated present queue */
		s_WindowContext = windowContext;
		s_Surface = cinew Surface(windowContext);

		uint32_t queueFamilyPropertiesCount{ 0U };
		vkGetPhysicalDeviceQueueFamilyProperties(
			s_PhysicalDevice,
			&queueFamilyPropertiesCount,
			nullptr);

		if (queueFamilyPropertiesCount == 0U)
		{
			CIN_CRITICAL("Failed to retrieve queue family properties");
			return false;
		}

		STL::Vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			s_PhysicalDevice,
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
					s_PhysicalDevice,
					queueFamilyIndex,
					s_Surface->GetHandle(),
					&presentationSupported));

				if (presentationSupported)
				{
					if(s_QueueFamilies.Present == QueueFamilies::Absent )
					{
						CIN_TRACE("--Found present queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
						s_QueueFamilies.Present = static_cast<int32_t>(queueFamilyIndex);
						s_QueueFamilies.PresentQueueCount = queueCount;
					}
				}

				if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					CIN_TRACE("--Found graphics queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
					s_QueueFamilies.Graphics = static_cast<int32_t>(queueFamilyIndex);
					s_QueueFamilies.GraphicsQueueCount = queueCount;
					continue;
				}

				if (queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					CIN_TRACE("--Found compute queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
					s_QueueFamilies.Compute = static_cast<int32_t>(queueFamilyIndex);
					s_QueueFamilies.ComputeQueueCount = queueCount;

					if(presentationSupported)
					{
						CIN_TRACE("--Found present queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
						s_QueueFamilies.Present = static_cast<int32_t>(queueFamilyIndex);
						s_QueueFamilies.PresentQueueCount = queueCount;
					}
					continue;
				}

				/* Used for fast memory copying operations */
				if (queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					CIN_TRACE("--Found transfer queue family with index {0} with count of {1} queues", queueFamilyIndex, queueCount);
					s_QueueFamilies.Transfer = static_cast<int32_t>(queueFamilyIndex);
					s_QueueFamilies.TransferQueueCount = queueCount;
					continue;
				}
			}
		}

		if (s_QueueFamilies.Present == QueueFamilies::Absent)
		{
			CIN_ERROR("Failed to find a suitable present family");
			return false;
		}

		if (s_QueueFamilies.Graphics == QueueFamilies::Absent)
		{
			CIN_ERROR("Failed to find a suitable graphics family");
			return false;
		}

		constexpr float defaultQueuePriority{ 1.0f };
		STL::Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
		if (PresentAndGraphicsFamiliesShared())
		{
			CIN_ASSERT(s_QueueFamilies.Graphics == s_QueueFamilies.Present);
			CIN_ASSERT(s_QueueFamilies.GraphicsQueueCount == s_QueueFamilies.PresentQueueCount);
			constexpr STL::Array<float, 2U> defaultQueuePriorities{ defaultQueuePriority, defaultQueuePriority };

			VkDeviceQueueCreateInfo graphicsQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ s_QueueFamilies.Graphics },
				.queueCount{ s_QueueFamilies.GraphicsQueueCount > 1U ? 2U : 1U }, /* If queue family is shared, attempt using a different queue if more than 1 is available */
				.pQueuePriorities{ s_QueueFamilies.GraphicsQueueCount > 1U ? defaultQueuePriorities.data() : &defaultQueuePriority },
			};

			deviceQueueCreateInfos.emplace_back(std::move(graphicsQueueCreateInfo));
		}
		else
		{
			VkDeviceQueueCreateInfo presentQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ static_cast<uint32_t>(s_QueueFamilies.Present) },
				.queueCount{ 1U },
				.pQueuePriorities{ &defaultQueuePriority },
			};

			deviceQueueCreateInfos.emplace_back(std::move(presentQueueCreateInfo));
			VkDeviceQueueCreateInfo graphicsQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ s_QueueFamilies.Graphics },
				.queueCount{ 1U },
				.pQueuePriorities{ &defaultQueuePriority },
			};

			deviceQueueCreateInfos.emplace_back(std::move(graphicsQueueCreateInfo));
		}

		/* Transfer queue */
		if (s_QueueFamilies.Transfer != QueueFamilies::Absent)
		{
			VkDeviceQueueCreateInfo transferQueueCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ s_QueueFamilies.Transfer },
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
			VK_CHECK(vkEnumerateDeviceLayerProperties(s_PhysicalDevice, &availableLayerCount, nullptr));
			if (availableLayerCount != 0U)
			{
				STL::Vector<VkLayerProperties> availableLayers(availableLayerCount);
				VK_CHECK(vkEnumerateDeviceLayerProperties(s_PhysicalDevice, &availableLayerCount, &availableLayers[0U]));

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
			VK_CHECK(vkEnumerateDeviceExtensionProperties(s_PhysicalDevice, nullptr, &availableExtensionCount, nullptr));

			if (availableExtensionCount == 0U && !requiredExtensions.empty())
			{
				CIN_CRITICAL("Requested vulkan extensions, but none are available");
				return false;
			}

			STL::Vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
			VK_CHECK(vkEnumerateDeviceExtensionProperties(s_PhysicalDevice, nullptr, &availableExtensionCount, &availableExtensions[0]));

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
					return false;
				}
			}
		}

		CIN_ASSERT(!deviceQueueCreateInfos.empty(), "No device queues requested");
		const VkDeviceCreateInfo deviceCreateInfo{
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
			s_PhysicalDevice,
			&deviceCreateInfo,
			s_Allocator,
			&s_LogicalDevice));

		if (PresentAndGraphicsFamiliesShared())
		{
			CIN_ASSERT(s_QueueFamilies.Graphics == s_QueueFamilies.Present);
			if (PresentAndGraphicsQueuesCanBeSeparate())
			{
				CIN_ASSERT(s_QueueFamilies.GraphicsQueueCount > 1U);
				vkGetDeviceQueue(
					s_LogicalDevice,
					s_QueueFamilies.Graphics,
					0U, /* Pick first queue in a queue family */
					&s_Queues.Graphics);

				vkGetDeviceQueue(
					s_LogicalDevice,
					s_QueueFamilies.Present,
					1U, /* Pick second queue in a queue family */
					&s_Queues.Present);
			}
			else
			{
				vkGetDeviceQueue(
					s_LogicalDevice,
					s_QueueFamilies.Graphics,
					0U, /* Pick first queue in a queue family */
					&s_Queues.Graphics);

				s_Queues.Present = s_Queues.Graphics;
			}
		}
		else
		{
			vkGetDeviceQueue(
				s_LogicalDevice,
				s_QueueFamilies.Graphics,
				0U,
				&s_Queues.Graphics);

			vkGetDeviceQueue(
				s_LogicalDevice,
				s_QueueFamilies.Present,
				0U,
				&s_Queues.Present);
		}

		/* Pick a transfer queue if available, if not - default to graphics queue */
		if (s_QueueFamilies.Transfer != QueueFamilies::Absent)
			vkGetDeviceQueue(
				s_LogicalDevice,
				s_QueueFamilies.Transfer,
				0U, /* Pick first queue in a queue family */
				&s_Queues.Transfer);
		else
		{
			CIN_WARN("Transfer queue family was not found. Defaulting transfer queue to a graphics queue");
			s_Queues.Transfer = s_Queues.Graphics;
			s_QueueFamilies.Transfer = s_QueueFamilies.Graphics;
		}

		CIN_ASSERT(s_Queues.Graphics != VK_NULL_HANDLE, "Graphics queue is invalid");
		CIN_ASSERT(s_Queues.Present != VK_NULL_HANDLE, "Present is invalid");
		CIN_ASSERT(s_Queues.Transfer != VK_NULL_HANDLE, "Transfer queue is invalid");

		/* Create graphics command pool */
		{
			const VkCommandPoolCreateInfo graphicsCommandPoolCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ s_QueueFamilies.Graphics },
			};

			VK_CHECK(vkCreateCommandPool(
				s_LogicalDevice,
				&graphicsCommandPoolCreateInfo,
				s_Allocator,
				&s_CommandPools.Graphics));
		}

		CIN_TRACE("Created graphics command pool from queue family {}", s_QueueFamilies.Graphics);
		/* Create a transfer command pool if transfer queue family is available, if not - default to graphics command pool */
		if (s_QueueFamilies.Transfer != s_QueueFamilies.Graphics)
		{
			/* Create transfer command pool */
			const VkCommandPoolCreateInfo transferCommandPoolCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.queueFamilyIndex{ s_QueueFamilies.Transfer },
			};

			VK_CHECK(vkCreateCommandPool(
				s_LogicalDevice,
				&transferCommandPoolCreateInfo,
				s_Allocator,
				&s_CommandPools.Transfer));
			
			CIN_TRACE("Created transfer command pool from queue family {}", s_QueueFamilies.Transfer);
		}
		else
			s_CommandPools.Transfer = s_CommandPools.Graphics;

		const auto [width, height] { windowContext->GetSize() };
		s_Swapchain = cinew Swapchain(width, height, s_Surface);

		return true;
	}

	void GraphicsContext::RecreateSurface()
	{
		CIN_ASSERT(s_WindowContext, "Invalid window context");
		Surface* oldSurface{ s_Surface };
		s_Surface = cinew Surface(s_WindowContext);

		const auto [width, height] { s_WindowContext->GetSize() };
		s_Swapchain->Recreate(width, height, s_Surface);
		
		cindel oldSurface;
	}

	void GraphicsContext::ResizeSwapchain()
	{
		CIN_ASSERT(s_WindowContext, "Invalid window context");
		/* No surface recreation */
		const auto [width, height] { s_WindowContext->GetSize() };
		s_Swapchain->Recreate(width, height, s_Surface);
	}

	void GraphicsContext::AcquireNextImage()
	{
		s_Swapchain->AcquireNextSwapchainImage();
	}

	void GraphicsContext::PresentImage()
	{
		s_Swapchain->PresentSwapchainImage();
	}

	uint32_t GraphicsContext::GetSwapchainImageCount()
	{
		return s_Swapchain->GetImageCount();
	}

	uint32_t GraphicsContext::GetMinimalSwapchainImageCount()
	{
		return s_Swapchain->GetMinimalImageCount();
	}

	uint32_t GraphicsContext::GetMaximumSwapchainImageCount()
	{
		return s_Swapchain->GetMaximumImageCount();
	}

	VkRenderPass GraphicsContext::GetSwapchainRenderPass()
	{
		return s_Swapchain->GetRenderPass();
	}
}