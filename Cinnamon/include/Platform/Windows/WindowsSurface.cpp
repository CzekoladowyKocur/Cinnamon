#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"

namespace Cinnamon {
	struct PlatformWindowState
	{
		HWND Handle{ nullptr };
		HINSTANCE Instance{ nullptr };

		uint32_t StyleFlags{ 0U };
		uint32_t ExtendedStyleFlags{ 0U };
	};

	Surface::Surface(const STL::Unique<Window>& windowContext) noexcept
		:
		m_WindowState(windowContext->GetState()),
		m_UseVSync(windowContext->GetProperties().UseVSync),
		m_DesiredPresentMode(m_UseVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR),
		m_Handle(VK_NULL_HANDLE)
	{
		CIN_ASSERT(m_WindowState, "Window state is invalid");

		const VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
			.pNext{ nullptr },
			.flags{ 0U },
			.hinstance{ m_WindowState->Instance },
			.hwnd{ m_WindowState->Handle },
		};

		VK_CHECK(vkCreateWin32SurfaceKHR(
			GraphicsContext::GetInstance(),
			&win32SurfaceCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Handle));
	}

	void Surface::Recreate()
	{
		CIN_ASSERT(m_Handle);
		vkDestroySurfaceKHR(
			GraphicsContext::GetInstance(),
			m_Handle,
			GraphicsContext::GetAllocator());

		CIN_ASSERT(m_WindowState, "Window state is invalid");
		const VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
			.pNext{ nullptr },
			.flags{ 0U },
			.hinstance{ m_WindowState->Instance },
			.hwnd{ m_WindowState->Handle },
		};

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