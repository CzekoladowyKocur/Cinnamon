#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Platform/Platform.hpp"

namespace Cinnamon {
	extern bool PlatformForceLinking;
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

	InternalScope VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objectType,
		size_t object,
		size_t location,
		int32_t messageCode,
		const char* layerPrefix,
		const char* message,
		void* userData)
	{
		CIN_UNUSED(flags);
		CIN_UNUSED(objectType);
		CIN_UNUSED(object);
		CIN_UNUSED(location);
		CIN_UNUSED(messageCode);
		CIN_UNUSED(userData);
		if(flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			CIN_WARN("Vulkan debug report callback [{}]: {}", layerPrefix, message);

		return VK_FALSE;
	}
#endif
	struct
	{
		VkInstance					Instance{ VK_NULL_HANDLE };
		VkPhysicalDevice			PhysicalDevice{ VK_NULL_HANDLE };
		VkAllocationCallbacks*		Allocator{ VK_NULL_HANDLE };
		/* Debug */
#ifdef CIN_DEBUG
		VkDebugUtilsMessengerEXT	DebugUtilitiesMessenger{ VK_NULL_HANDLE };
		VkDebugReportCallbackEXT	DebugReportCallback{ VK_NULL_HANDLE };
#endif
	} constinit InternalScope s_GraphicsContext;

	namespace GraphicsContext
	{
		Errr Initialize()
		{
			/* Initialize volk to load vulkan functions */
			if (volkInitialize() != VK_SUCCESS)
			{
				CIN_CRITICAL("Failed to initialize volk");
				return Error::Failure;
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
					return Error::Failure;
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
						return Error::Failure;
					}
				}

				const VkApplicationInfo applicationInfo
				{
					.sType{ VK_STRUCTURE_TYPE_APPLICATION_INFO },
					.pNext{ nullptr },
					.pApplicationName{ "Application" },
					.applicationVersion{ VK_MAKE_VERSION(0, 0, 1) },
					.pEngineName{ "Cinnamon" },
					.engineVersion{ VK_MAKE_VERSION(0, 0, 1) },
					.apiVersion{ GetAPIVersion() },
				};

				const VkInstanceCreateInfo instanceCreateInfo
				{
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
					s_GraphicsContext.Allocator,
					&s_GraphicsContext.Instance));

				volkLoadInstance(s_GraphicsContext.Instance);
#ifdef CIN_DEBUG
				constexpr VkDebugUtilsMessengerCreateInfoEXT debugUtilitiesMessengerCreateInfo
				{
					.sType{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT },
					.pNext{ nullptr },
					.flags{ 0U },
					.messageSeverity
					{
						VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
						VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT	|
						VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
					},
					.messageType
					{
						VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT		|
						VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT	|
						VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
					},
					.pfnUserCallback{ VulkanDebugUtilitiesCallback },
					.pUserData{ nullptr },
				};

				const PFN_vkCreateDebugUtilsMessengerEXT _vkCreateDebugUtilsMessengerEXT{ reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
						s_GraphicsContext.Instance,
						"vkCreateDebugUtilsMessengerEXT")) };

				CIN_ASSERT(_vkCreateDebugUtilsMessengerEXT);
				VK_CHECK(_vkCreateDebugUtilsMessengerEXT(
					s_GraphicsContext.Instance,
					&debugUtilitiesMessengerCreateInfo,
					s_GraphicsContext.Allocator,
					&s_GraphicsContext.DebugUtilitiesMessenger));

				constexpr VkDebugReportCallbackCreateInfoEXT debugUtilsMessengerCreateInfo
				{
					.sType{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT },
					.pNext{ nullptr },
					.flags
					{
						VK_DEBUG_REPORT_WARNING_BIT_EXT |
						VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
						VK_DEBUG_REPORT_ERROR_BIT_EXT |
						VK_DEBUG_REPORT_DEBUG_BIT_EXT
					},
					.pfnCallback{ VulkanDebugReportCallback },
					.pUserData{ nullptr }
				};
				
				const PFN_vkCreateDebugReportCallbackEXT _vkCreateDebugReportCallback{ reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(
						s_GraphicsContext.Instance,
						"vkCreateDebugReportCallbackEXT")) };

				CIN_ASSERT(_vkCreateDebugReportCallback);
				VK_CHECK(_vkCreateDebugReportCallback(
					s_GraphicsContext.Instance, 
					&debugUtilsMessengerCreateInfo, 
					nullptr, 
					&s_GraphicsContext.DebugReportCallback));
#endif
				uint32_t physicalDeviceCount{ 0 };
				VK_CHECK(vkEnumeratePhysicalDevices(s_GraphicsContext.Instance, &physicalDeviceCount, nullptr));
				STL::Vector<VkPhysicalDevice> availablePhysicalDevices(physicalDeviceCount);
				VK_CHECK(vkEnumeratePhysicalDevices(s_GraphicsContext.Instance, &physicalDeviceCount, &availablePhysicalDevices[0]));

				if (availablePhysicalDevices.empty())
				{
					CIN_CRITICAL("Failed to find a supported physical device");
					return Error::Failure;
				}

				for (uint32_t i{ 0 }; i < availablePhysicalDevices.size(); ++i)
				{
					const VkPhysicalDevice physicalDevice{ availablePhysicalDevices[i] };

					if (PhysicalDeviceMeetsRequirements(physicalDevice))
					{
						s_GraphicsContext.PhysicalDevice = physicalDevice;
						break;
					}
				}

				/* If no discrete gpu was found, pick the first one */
				if (!s_GraphicsContext.PhysicalDevice)
					s_GraphicsContext.PhysicalDevice = availablePhysicalDevices.front();

				CIN_TRACE("Initialized graphics context");
			}

			return Error::Success;
		}

		void Shutdown()
		{
#ifdef CIN_DEBUG
			const PFN_vkDestroyDebugUtilsMessengerEXT _vkDestroyDebugUtilsMessengerEXT{ reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(s_GraphicsContext.Instance, "vkDestroyDebugUtilsMessengerEXT")) };
			CIN_ASSERT(_vkDestroyDebugUtilsMessengerEXT != nullptr);

			_vkDestroyDebugUtilsMessengerEXT(
				s_GraphicsContext.Instance,
				s_GraphicsContext.DebugUtilitiesMessenger,
				s_GraphicsContext.Allocator);

			const PFN_vkDestroyDebugReportCallbackEXT _vkDestroyDebugReportCallbackEXT{ reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(s_GraphicsContext.Instance, "vkDestroyDebugReportCallbackEXT")) };
			CIN_ASSERT(_vkDestroyDebugUtilsMessengerEXT != nullptr);

			_vkDestroyDebugReportCallbackEXT(
				s_GraphicsContext.Instance,
				s_GraphicsContext.DebugReportCallback,
				s_GraphicsContext.Allocator);
#endif
			vkDestroyInstance(
				s_GraphicsContext.Instance,
				s_GraphicsContext.Allocator);
#ifdef CIN_PLATFORM_WINDOWS
			const HMODULE sharedVulkanLibraryModule{ GetModuleHandleA("vulkan-1.dll") };
			CIN_ASSERT(sharedVulkanLibraryModule, "Invalid module");
			CIN_VERIFY(FreeLibrary(sharedVulkanLibraryModule));
#else
			CIN_WARN("Vulkan library was left unloaded");
#endif
			if (PlatformForceLinking)
			{
				vkBindImageMemory2(static_cast<VkDevice>(VK_NULL_HANDLE), 0U, nullptr);
				vkBindBufferMemory2(static_cast<VkDevice>(VK_NULL_HANDLE), 0U, nullptr);
				vkGetBufferMemoryRequirements2(static_cast<VkDevice>(VK_NULL_HANDLE), nullptr, nullptr);
				vkGetPhysicalDeviceMemoryProperties2(static_cast<VkPhysicalDevice>(VK_NULL_HANDLE), nullptr);
				vkGetImageMemoryRequirements2(static_cast<VkDevice>(VK_NULL_HANDLE), nullptr, nullptr);
			}
		}

		uint32_t GetAPIVersion()
		{
			return VK_API_VERSION_1_3;
		}

		VkInstance GetInstance()
		{
			CIN_ASSERT(s_GraphicsContext.Instance);
			return reinterpret_cast<VkInstance>(s_GraphicsContext.Instance);
		}

		VkPhysicalDevice GetPhysicalDevice()
		{
			CIN_ASSERT(s_GraphicsContext.PhysicalDevice);
			return s_GraphicsContext.PhysicalDevice;
		}

		VkAllocationCallbacks* GetAllocator()
		{
			// CIN_ASSERT(s_GraphicsContext.Allocator)
			return nullptr;
		}
	}
}