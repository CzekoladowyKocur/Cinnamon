#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Renderer/Surface.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"

#include <wayland-client.h>
extern "C"
{
#include "ThirdParty/xdg/xdg-shell-unstable-v6.h"
}

#define VOLK_IMPLEMENTATION
#include "ThirdParty/volk/volk.h"

namespace Cinnamon {
	struct PlatformWindowState
	{
	    // Globals

	    wl_display* wlDisplay { nullptr };
	    wl_compositor* wlCompositor { nullptr };
	    wl_registry* wlRegistry { nullptr };
	    zxdg_shell_v6* xdgShell { nullptr };
	    wl_output* wlOutput { nullptr };
	    wl_seat* wlSeat { nullptr };

	    // Objects

		/* Surfaces */
	    wl_surface* wlSurface { nullptr };
	    zxdg_surface_v6* xdgSurface { nullptr };
	    zxdg_toplevel_v6* xdgToplevel { nullptr };

		/* Input */
		wl_keyboard* wlKeyboard { nullptr };
	};

	Surface::Surface(const Window* windowContext) noexcept
		:
		m_WindowState(windowContext->GetState()),
		m_Handle(VK_NULL_HANDLE)
	{
		CIN_ASSERT(m_WindowState, "Window state is invalid");

		VkWaylandSurfaceCreateInfoKHR waylandSurfaceCreateInfo;
		waylandSurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		waylandSurfaceCreateInfo.display = m_WindowState->wlDisplay;
		waylandSurfaceCreateInfo.surface = m_WindowState->wlSurface;
		waylandSurfaceCreateInfo.flags = 0;
		waylandSurfaceCreateInfo.pNext = nullptr;

		VK_CHECK(vkCreateWaylandSurfaceKHR(
			GraphicsContext::GetInstance(),
			&waylandSurfaceCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Handle));
	}

	Surface::~Surface() noexcept
	{
		vkDestroySurfaceKHR(
			GraphicsContext::GetInstance(),
			m_Handle,
			GraphicsContext::GetAllocator());
	}

	VkSurfaceKHR Surface::GetHandle() const
	{
		CIN_ASSERT(m_Handle, "Invalid surface handle");
		return m_Handle;
	}
}
#endif