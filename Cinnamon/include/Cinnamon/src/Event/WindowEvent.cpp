#include "Cinnamon/include/Event/WindowEvent.h"

namespace Cinnamon {
	WindowEvent::WindowEvent(const Window* const windowHandle) noexcept
		:
		m_WindowHandle(windowHandle)
	{}

	const Window* WindowEvent::GetWindowHandle() const noexcept
	{
		return m_WindowHandle;
	}

	WindowClosedEvent::WindowClosedEvent(const Window* const windowHandle) noexcept
		:
		WindowEvent(windowHandle)
	{}

	WindowResizedEvent::WindowResizedEvent(const Window* const windowHandle, const uint32_t width, const uint32_t height) noexcept
		:
		WindowEvent(windowHandle),
		m_Width(width),
		m_Height(height)
	{}

	uint32_t WindowResizedEvent::GetWidth() const
	{
		return m_Width;
	}

	uint32_t WindowResizedEvent::GetHeight() const
	{
		return m_Height;
	}

	std::pair<uint32_t, uint32_t> WindowResizedEvent::GetResize() const
	{
		return { m_Width, m_Height };
	}

	WindowMinimizedEvent::WindowMinimizedEvent(const Window* const windowHandle) noexcept
		:
		WindowEvent(windowHandle)
	{}

	WindowMaximizedEvent::WindowMaximizedEvent(const Window* const windowHandle) noexcept
		:
		WindowEvent(windowHandle)
	{}

	WindowSurfaceUpdatedEvent::WindowSurfaceUpdatedEvent(const Window* const windowHandle) noexcept
		:
		WindowEvent(windowHandle)
	{}
}