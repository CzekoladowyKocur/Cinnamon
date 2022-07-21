#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Window.h"
#include <wayland-client.h>
#include "ThirdParty/xdg/xdg-shell-unstable-v6.h"
#include <iostream>

namespace Cinnamon {
	/* Declared in Window.h */
	struct PlatformWindowState
	{
        wl_display* display {nullptr};
        wl_compositor* compositor {nullptr};
        wl_registry* registry {nullptr};
        wl_shell* shell {nullptr}; 
        wl_shm* shm {nullptr}; 
	};

	Window::Window(WindowProperties&& windowProperties) noexcept
		:
		m_Properties(std::move(windowProperties))
	{
        std::cout << "Heyy\n";
        m_State = new PlatformWindowState;
        m_State->display = wl_display_connect(NULL);
    }

	Window::~Window() noexcept
	{}

	uint32_t Window::GetHeight() const
    {
        return 100;
    }
}
#endif