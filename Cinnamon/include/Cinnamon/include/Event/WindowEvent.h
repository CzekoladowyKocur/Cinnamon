#pragma once
#include "Cinnamon/include/Event/Event.h"

namespace Cinnamon {
	class Window;

	class WindowEvent : public Event
	{
	private:
	public:
		explicit WindowEvent(const Window* const windowHandle) noexcept;
		virtual ~WindowEvent() noexcept = default;

		virtual const Window* GetWindowHandle() const noexcept final;
	protected:
		const Window* const m_WindowHandle;
	};

	class WindowClosedEvent final : public WindowEvent
	{
	private:
	public:
		explicit WindowClosedEvent(const Window* const windowHandle) noexcept;
		virtual ~WindowClosedEvent() noexcept = default;

		EVENT_TYPE(WindowClosed)
		EVENT_CATEGORY(WindowSurface)
	private:
	};

	class WindowMinimizedEvent final : public WindowEvent
	{
	private:
	public:
		explicit WindowMinimizedEvent(const Window* const windowHandle) noexcept;
		virtual ~WindowMinimizedEvent() noexcept = default;

		EVENT_TYPE(WindowMinimized)
		EVENT_CATEGORY(WindowSurface)
	private:
	};

	class WindowMaximizedEvent final : public WindowEvent
	{
	private:
		explicit WindowMaximizedEvent(const Window* const windowHandle) noexcept;
		virtual ~WindowMaximizedEvent() noexcept = default;

		EVENT_TYPE(WindowMaximized)
		EVENT_CATEGORY(WindowSurface)
	public:
	private:
	};

	class WindowResizedEvent final : public WindowEvent
	{
	private:
	public:
		explicit WindowResizedEvent(const Window* const windowHandle, const uint32_t width, const uint32_t height) noexcept;
		virtual ~WindowResizedEvent() noexcept = default;

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		std::pair<uint32_t, uint32_t> GetResize() const;
		
		EVENT_TYPE(WindowResized)
		EVENT_CATEGORY(WindowSurface)
	private:
		uint32_t m_Width, m_Height;
	};

	class WindowSurfaceUpdatedEvent final : public WindowEvent
	{
	private:
	public:
		explicit WindowSurfaceUpdatedEvent(const Window* const windowHandle) noexcept;
		virtual ~WindowSurfaceUpdatedEvent() noexcept = default;

		EVENT_TYPE(WindowSurfaceUpdated)
		EVENT_CATEGORY(WindowSurface)
	private:
	};
}