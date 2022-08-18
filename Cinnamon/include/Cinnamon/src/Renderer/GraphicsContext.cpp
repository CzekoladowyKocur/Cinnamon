#include "Cinnamon/include/Renderer/GraphicsContext.h"
#include "Cinnamon/include/Renderer/Surface.h"
#include "Cinnamon/include/Renderer/Swapchain.h"
#include "Cinnamon/include/Core/Window.h"
#include "Platform/Platform.h"

namespace Cinnamon {
	struct DrawContext
	{
		Surface* SurfaceContext;
		Swapchain* SwapchainContext;
	};

	InternalScope STL::UMap<const Window*, DrawContext> s_ContextMap;
	/* TODO: Change to GPU score system */
	InternalScope bool PhysicalDeviceMeetsRequirements(const VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		return physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	}
#if CIN_DEBUG
	InternalScope VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		uint64_t object,
		uint64_t location,
		int32_t messageCode,
		const char* pLayerPrefix,
		const char* pMessage,
		void* pUserData)
	{
		CIN_UNUSED(object);
		CIN_UNUSED(location);
		CIN_UNUSED(messageCode);
		CIN_UNUSED(pLayerPrefix);
		CIN_UNUSED(pUserData);
		/* TODO: Remove */
		CIN_UNUSED(pMessage);
		CIN_UNUSED(objectType);

		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			CIN_INFO("Vulkan:\n  Object: {0}\n  Message: {1}", static_cast<int>(objectType), pMessage);

		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			CIN_WARN("Vulkan:\n  Object: {0}\n  Message: {1}", static_cast<int>(objectType), pMessage);

		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			CIN_WARN("Vulkan:\n  Object: {0}\n  Message: {1}", static_cast<int>(objectType), pMessage);

		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			CIN_ERROR("Vulkan:\n  Object: {0}\n  Message: {1}", static_cast<int>(objectType), pMessage);

		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			CIN_WARN("Vulkan:\n  Object: {0}\n  Message: {1}", static_cast<int>(objectType), pMessage);

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
		const auto _vkCreateDebugReportCallbackEXT{ (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(s_Instance, "vkCreateDebugReportCallbackEXT") };
		CIN_ASSERT(_vkCreateDebugReportCallbackEXT != nullptr);
		
		const VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT },
			.pNext{ nullptr },
			.flags{ 0U },
			.pfnCallback{ VulkanDebugReportCallback },
			.pUserData{ nullptr },
		};

		_vkCreateDebugReportCallbackEXT(s_Instance, &debugReportCreateInfo, s_Allocator, &s_DebugObject);
#endif
		uint32_t physicalDeviceCount{ 0 };
		VK_CHECK(vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, nullptr));
		STL::Vector<VkPhysicalDevice> availablePhysicalDevices(physicalDeviceCount);
		VK_CHECK(vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, &availablePhysicalDevices[0]));

		if (availablePhysicalDevices.empty())
		{
			CIN_CRITICAL("Failed to find a physical device");
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

		s_ContextMap.clear();
		CIN_TRACE("Initialized graphics context");
		return true;
	}

	bool GraphicsContext::Shutdown()
	{
		VK_CHECK(vkDeviceWaitIdle(s_LogicalDevice));
		for (const auto& [window, drawContext] : s_ContextMap)
		{
			CIN_UNUSED(window);
			cindel drawContext.SwapchainContext;
			cindel drawContext.SurfaceContext;
		}

		vkDestroyDevice(
			s_LogicalDevice,
			s_Allocator);
#ifdef CIN_DEBUG
		const auto _vkDestroyDebugReportCallbackEXT{ (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(s_Instance, "vkDestroyDebugReportCallbackEXT") };
		CIN_ASSERT(_vkDestroyDebugReportCallbackEXT != nullptr);

		_vkDestroyDebugReportCallbackEXT(
			s_Instance,
			s_DebugObject,
			s_Allocator);
#endif
		vkDestroyInstance(
			s_Instance,
			s_Allocator);

		return true;
	}

	bool GraphicsContext::CreateSurface(const Window* const windowContext)
	{
		uint32_t queueFamilyPropertiesCount{ 0 };
		vkGetPhysicalDeviceQueueFamilyProperties(
			s_PhysicalDevice,
			&queueFamilyPropertiesCount,
			nullptr);

		if (queueFamilyPropertiesCount == 0)
		{
			CIN_CRITICAL("Failed to retrieve queue family properties");
			return false;
		}

		STL::Vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(
			s_PhysicalDevice,
			&queueFamilyPropertiesCount,
			&queueFamilyProperties[0]);

		CIN_WARN("Add present queue support");
		for (uint32_t i{ 0 }; i < queueFamilyProperties.size(); ++i)
		{
			const VkQueueFamilyProperties& queueFamily{ queueFamilyProperties[i] };

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				CIN_TRACE("Found graphics queue with index {0}", i);
				s_QueueFamilies.Graphics = i;
				continue;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				CIN_TRACE("Found compute queue with index {0}", i);
				s_QueueFamilies.Compute = i;
				continue;
			}

			if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				CIN_TRACE("Found transer queue with index {0}", i);
				s_QueueFamilies.Transfer = i;
				continue;
			}
		}

		/* Graphics only for now */
		constexpr float queuePriority{ 1.0f };
		const VkDeviceQueueCreateInfo queueCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.queueFamilyIndex{ static_cast<uint32_t>(s_QueueFamilies.Graphics) },
			.queueCount{ 1U },
			.pQueuePriorities{ &queuePriority },
		};

		const STL::Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos{ queueCreateInfo };
		/* None for now */
		const VkPhysicalDeviceFeatures enabledFeatures{};

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
				VK_CHECK(vkEnumerateDeviceLayerProperties(s_PhysicalDevice, &availableLayerCount, &availableLayers[0]));

				/* Continues program even if requested layer isn't supported */
				for (uint32_t i{ 0U }; i < requestedLayers.size(); ++i)
				{
					bool found{ false };
					CIN_TRACE("Requested layer: {0}", requestedLayers[i]);
					for (uint32_t j{ 0U }; j < availableLayers.size(); ++j)
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
				CIN_TRACE("Requested extension: {0}", requiredExtensions[i]);
				for (uint32_t j{ 0U }; j < availableExtensions.size(); ++j)
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

		CIN_ASSERT(not deviceQueueCreateInfos.empty(), "No device queues requested");
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

		/* Pick a single graphics queue for now */
		CIN_TRACE("Picking first graphics queue from family index {0}", s_QueueFamilies.Graphics);
		vkGetDeviceQueue(
			s_LogicalDevice,
			s_QueueFamilies.Graphics,
			0U, /* Pick first queue */
			&s_Queues.Graphics);

		CIN_ASSERT(s_Queues.Graphics != VK_NULL_HANDLE, "Failed to pick graphics queue");

		Surface* surf{ cinew Surface(windowContext) };
		const auto [width, height] { windowContext->GetSize() };
		s_ContextMap[windowContext] = { surf, cinew Swapchain(width, height, surf->GetHandle()) };

		return true;
	}

	void GraphicsContext::ResizeSurface(const Window* const windowContext, const uint32_t width, const uint32_t height)
	{
		Swapchain*& swapchain = s_ContextMap[windowContext].SwapchainContext;
		Surface*& surface = s_ContextMap[windowContext].SurfaceContext;

		Surface* oldSurface{ surface };
		surface = cinew Surface(windowContext);
		swapchain->Recreate(width, height, surface->GetHandle());
		
		cindel oldSurface;
	}

	void GraphicsContext::ResizeSurface(const Swapchain* const swapchain)
	{
		for (const auto& [window, drawContext] : s_ContextMap)
			if (drawContext.SwapchainContext == swapchain)
			{
				const auto [width, height] { window->GetSize() };
				ResizeSurface(window, width, height);
			}
	}

	void GraphicsContext::AcquireNextImage(const Window* const windowContext)
	{
		s_ContextMap[windowContext].SwapchainContext->AcquireNextSwapchainImage();
	}

	void GraphicsContext::PresentImage(const Window* const windowContext)
	{
		s_ContextMap[windowContext].SwapchainContext->PresentSwapchainImage();
	}
}