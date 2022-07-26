#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Renderer/Surface.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"

namespace Cinnamon {
	struct PlatformWindowState
	{
		HWND Handle{ nullptr };
		HINSTANCE Instance{ nullptr };

		uint32_t StyleFlags{ 0U };
		uint32_t ExtendedStyleFlags{ 0U };
	};

	Surface::Surface(const Window* windowContext) noexcept
		:
		m_WindowState(windowContext->GetState()),
		m_Handle(VK_NULL_HANDLE)
	{
		CIN_ASSERT(m_WindowState, "Window state is invalid");

		VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo;
		win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32SurfaceCreateInfo.hinstance = m_WindowState->Instance;
		win32SurfaceCreateInfo.hwnd = m_WindowState->Handle;
		win32SurfaceCreateInfo.flags = 0;
		win32SurfaceCreateInfo.pNext = nullptr;

		VK_CHECK(vkCreateWin32SurfaceKHR(
			GraphicsContext::GetInstance(),
			&win32SurfaceCreateInfo,
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