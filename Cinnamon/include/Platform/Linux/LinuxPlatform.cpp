#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/TypeDefines.hpp"
#include "Cinnamon/include/Core/Core.hpp"

/* constexpr char C_FOREGROUND_BLACK[]				{ "\033[0;30m" }; */
constexpr char C_FOREGROUND_RED[]					{ "\033[0;31m" };
/* constexpr char C_FOREGROUND_GREEN[]				{ "\033[0;32m" }; */
constexpr char C_FOREGROUND_YELLOW[]				{ "\033[0;33m" };
/* constexpr char C_FOREGROUND_BLUE[]				{ "\033[0;34m" };
constexpr char C_FOREGROUND_MAGENTA[]				{ "\033[0;35m" };
constexpr char C_FOREGROUND_CYAN[]					{ "\033[0;36m" };
constexpr char C_FOREGROUND_WHITE[]					{ "\033[0;37m" }; */

constexpr char C_FOREGROUND_BRIGHTBLACK[]			{ "\033[0;90m" };
constexpr char C_FOREGROUND_BRIGHTRED[]				{ "\033[0;91m" };
/* constexpr char C_FOREGROUND_BRIGHTGREEN[]		{ "\033[0;92m" };
constexpr char C_FOREGROUND_BRIGHTYELLOW[]			{ "\033[0;93m" };
constexpr char C_FOREGROUND_BRIGHTBLUE[]			{ "\033[0;94m" };
constexpr char C_FOREGROUND_BRIGHTMAGENTA[]			{ "\033[0;95m" };
constexpr char C_FOREGROUND_BRIGHTCYAN[]			{ "\033[0;96m" };
constexpr char C_FOREGROUND_BRIGHTWHITE[]			{ "\033[0;97m" }; */

constexpr char C_FOREGROUND_DEFAULT[]				{ "\033[0;0m"  };

namespace Cinnamon {
	namespace Platform {
		Errr Initialize()
		{
        	return Error::Success;
		}

		void Shutdown()
		{}

		double GetAbsoluteTime()
		{
			timespec timeSpec;
			clock_gettime(CLOCK_REALTIME, &timeSpec);

			return static_cast<double>(timeSpec.tv_sec + static_cast<double>(timeSpec.tv_nsec) / 1'000'000'000);
		}

		/* Vulkan */
		STL::Vector<const char*> GetRequiredVulkanExtensions()
		{
			return 
			{
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

		STL::Vector<const char*> GetRequestedVulkanLayers()
		{
			return 
			{
#ifdef CIN_DEBUG
				"VK_LAYER_KHRONOS_validation",
#endif
			};
		}

		STL::Vector<const char*> GetRequiredVulkanDeviceExtensions()
		{
			return 
			{
				"VK_KHR_get_memory_requirements2",
				"VK_KHR_swapchain",
			};
		}

		STL::Vector<const char*> GetRequestedVulkanDeviceLayers()
		{
			return {};
		}

		void WriteToConsole(const char* message, const EConsoleTextColor color)
		{
			/* Kinda reflected off EConsoleTextColor - Magenta is big no no */
			constexpr const char* colors[5]
			{
				C_FOREGROUND_DEFAULT,
				C_FOREGROUND_BRIGHTBLACK,
				C_FOREGROUND_YELLOW,
				C_FOREGROUND_BRIGHTRED,
				C_FOREGROUND_RED
			};

			printf("%s%s%s", colors[static_cast<size_t>(color)], message, "\033[0m");
		}

		STL::String GetBuildDate()
		{
			return CIN_TIMESTAMP;
		}		
	}
}
#endif