#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Renderer/Surface.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"

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

	Surface::Surface(const Window* const windowContext) noexcept
		:
		m_WindowState(windowContext->GetState()),
		m_UseVSync(windowContext->GetProperties().UseVSync),
		m_DesiredPresentMode(m_UseVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR),
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

	VkPresentModeKHR Surface::GetDesiredPresentMode() const
	{
		return m_DesiredPresentMode;
	}

	VkSurfaceKHR Surface::GetHandle() const
	{
		CIN_ASSERT(m_Handle, "Invalid surface handle");
		return m_Handle;
	}
}
#endif