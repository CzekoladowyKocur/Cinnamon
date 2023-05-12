#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"

#include <wayland-client.h>
extern "C"
{
/* #include "ThirdParty/xdg/xdg-shell-unstable-v6.h" */
#include "ThirdParty/xdg/xdg-shell.h"
}

#define VOLK_IMPLEMENTATION
#include "ThirdParty/volk/volk.h"

namespace Cinnamon {

	struct PointerEvent
	{
		uint32_t eventMask;
		wl_fixed_t x, y;
		uint32_t button, state;
		uint32_t serial;
		struct {
			bool valid;
			wl_fixed_t value;
			int32_t discrete;
		} axes[2];
		uint32_t axisSource;
	};

	struct PlatformWindowState
	{
	    // Globals

	    wl_display* wlDisplay { nullptr };
	    wl_compositor* wlCompositor { nullptr };
	    wl_registry* wlRegistry { nullptr };
	    xdg_wm_base* xdgWMBase { nullptr };
	    wl_output* wlOutput { nullptr };
	    wl_seat* wlSeat { nullptr };

	    // Objects

		/* Surfaces */
	    wl_surface* wlSurface { nullptr };
	    xdg_surface* xdgSurface { nullptr };
	    xdg_toplevel* xdgToplevel { nullptr };

		/* Input */
		wl_keyboard* wlKeyboard { nullptr };
		wl_pointer* wlPointer { nullptr };
		PointerEvent pointerEvent;
	};

	namespace Platform 
	{
		/* Surface -> [Window context, desired present mode] */
		InternalScope STL::UMap<VkSurfaceKHR, std::pair<const PlatformWindowState*, VkPresentModeKHR>> s_SurfaceMap;

		VkSurfaceKHR CreateWindowSurface(const STL::Unique<Window>& window)
		{
			CIN_ASSERT(window);
			const PlatformWindowState* const windowState{ window->GetState() };

			CIN_ASSERT(windowState, "Window state is invalid");

			VkWaylandSurfaceCreateInfoKHR waylandSurfaceCreateInfo;
			waylandSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
			waylandSurfaceCreateInfo.display = windowState->wlDisplay;
			waylandSurfaceCreateInfo.surface = windowState->wlSurface;
			waylandSurfaceCreateInfo.flags = 0;
			waylandSurfaceCreateInfo.pNext = nullptr;

			VkSurfaceKHR surface{ VK_NULL_HANDLE };
			VK_CHECK(vkCreateWaylandSurfaceKHR(
				GraphicsContext::GetInstance(),
				&waylandSurfaceCreateInfo,
				GraphicsContext::GetAllocator(),
				&surface));

			const WindowProperties& windowProperties{ window->GetProperties() };
			CIN_ASSERT(!s_SurfaceMap.contains(surface));
			s_SurfaceMap[surface] = std::make_pair(windowState, windowProperties.UseVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR);
			return surface;
		}

		VkSurfaceKHR RetrieveWindowSurface(const STL::Unique<Window>& window)
		{
			/* Check if surface was already created by the device */
			for (const auto& [surface, surfaceState] : s_SurfaceMap)
			{
				const auto& [windowState, desiredPresentMode] { surfaceState};

				if (window->GetState() == windowState)
					return surface;
			}

			return CreateWindowSurface(window);
		}

		VkSurfaceKHR RecreateWindowSurface(const VkSurfaceKHR surface)
		{
			CIN_ASSERT(s_SurfaceMap.contains(surface));
			
			vkDestroySurfaceKHR(
				GraphicsContext::GetInstance(),
				surface,
				GraphicsContext::GetAllocator());

			const PlatformWindowState* const windowState{ s_SurfaceMap[surface].first };
			VkWaylandSurfaceCreateInfoKHR waylandSurfaceCreateInfo;
			waylandSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
			waylandSurfaceCreateInfo.display = windowState->wlDisplay;
			waylandSurfaceCreateInfo.surface = windowState->wlSurface;
			waylandSurfaceCreateInfo.flags = 0;
			waylandSurfaceCreateInfo.pNext = nullptr;

			VkSurfaceKHR newSurface{ VK_NULL_HANDLE };
			VK_CHECK(vkCreateWaylandSurfaceKHR(
				GraphicsContext::GetInstance(),
				&waylandSurfaceCreateInfo,
				GraphicsContext::GetAllocator(),
				&newSurface));

			return newSurface;
		}

		VkPresentModeKHR GetDesiredSurfacePresentMode(const VkSurfaceKHR surface)
		{
			CIN_ASSERT(s_SurfaceMap.contains(surface));
			return s_SurfaceMap[surface].second;
		}

		void DestroySurface(const VkSurfaceKHR surface)
		{
			CIN_ASSERT(s_SurfaceMap.contains(surface));

			vkDestroySurfaceKHR(
				GraphicsContext::GetInstance(),
				surface,
				GraphicsContext::GetAllocator());

			s_SurfaceMap.erase(surface);
		}
	}	
}
#endif