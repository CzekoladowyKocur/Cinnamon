#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Core.h"

namespace Cinnamon {

	bool Platform::Initialize()
	{
        return true;
	}

	bool Platform::Shutdown()
	{
		return true;
	}

	double Platform::GetAbsoluteTime()
	{
		CIN_UNIMPLEMENTED();
		return 0.0;
	}

	/* Vulkan */
	STL::Vector<const char*> Platform::GetRequiredVulkanExtensions()
	{
		return {
			"VK_KHR_surface",
			"VK_KHR_wayland_surface",
			"VK_KHR_get_physical_device_properties2",
			/* "VK_KHR_bind_memory2", core in 1.1 */
			/* VK_KHR_dedicated_allocation, core in 1.1 */
#ifdef CIN_DEBUG
			"VK_EXT_debug_report",
			"VK_EXT_debug_utils",
#endif
		};
	}

	STL::Vector<const char*> Platform::GetRequestedVulkanLayers()
	{
		return {
#ifdef CIN_DEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
		};
	}

	STL::Vector<const char*> Platform::GetRequiredVulkanDeviceExtensions()
	{
		return {
			"VK_KHR_swapchain",
		};
	}

	STL::Vector<const char*> Platform::GetRequestedVulkanDeviceLayers()
	{
		return {};
	}

	void Platform::WriteToConsole(const char* message, const EConsoleTextColor color)
	{
        CIN_UNUSED(color); printf(message);
	}

	[[nodiscard]] STL::String Platform::GetBuildDate()
	{
		return CIN_TIMESTAMP;
	}
}
#endif