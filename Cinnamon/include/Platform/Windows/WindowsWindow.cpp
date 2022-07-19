#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Core/Window.h"
#include <Windows.h>

namespace Cinnamon {
	/* Declared in Window.h */
	struct PlatformWindowState
	{
		HWND Handle{ nullptr };
		HINSTANCE Instance{ nullptr };
		
		// uint32_t StyleFlags{0};
		// uint32_t ExtendedStyleFlags{0};
	};

	Window::Window(WindowProperties&& windowProperties) noexcept
		:
		m_Properties(windowProperties)
	{}

	Window::~Window() noexcept
	{}
}
#endif