#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"
#include "Cinnamon/include/Event/KeyEvent.h"

#include <wayland-client.h>
extern "C"
{
#include "ThirdParty/xdg/xdg-shell-unstable-v6.h"
}

#define wl_array_for_each_casted(pos, array, type) \
	for \
    (pos = reinterpret_cast<type>((array)->data); \
    (const char *) pos < ((const char *) (array)->data + (array)->size); \
    (pos)++)

/* Todo: Manage closing the window and destroying this object properly */

namespace Cinnamon {

	struct {
		uint32_t width;
		uint32_t height;

		EWindowMode mode;
		bool focused;
	} InternalScope pending;

	/* Declared in Window.h */
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

	// -- wayland surface
	InternalScope void wlSurfaceEnter
	(
	    void* data,
	    wl_surface* wlSurface,
	    wl_output* output
	)
	{
	    /* Todo: implement */
	    (void)data;
	    (void)wlSurface;
	    (void)output;
	}

	InternalScope void wlSurfaceLeave
	(
	    void* data,
	    wl_surface* wlSurface,
	    wl_output* output
	)
	{
	    /* Todo: implement */
	    (void)data;
	    (void)wlSurface;
	    (void)output;
	}

	InternalScope constexpr wl_surface_listener wlSurfaceListener
	{
	    .enter = wlSurfaceEnter,
	    .leave = wlSurfaceLeave 
	};
	// wayland surface --

	// -- frame callback
	InternalScope void FrameCallback(void* data, wl_callback* frameCallback, uint32_t time);

	InternalScope constexpr wl_callback_listener wlSurfaceFrameListener
	{
	    .done = FrameCallback
	};

	InternalScope void FrameCallback(void* data, wl_callback* callback, uint32_t time)
	{
	    /* Todo: properly manage frame callback */

	    (void)time;

	    wl_callback_destroy(callback);

		Window* window = reinterpret_cast<Window*>(data);
		const PlatformWindowState* windowState { window->GetState() };

	    wl_callback* newCallback = wl_surface_frame(windowState->wlSurface);
	    wl_callback_add_listener(newCallback, &wlSurfaceFrameListener, window);

        ApplicationRenderEvent event;
        window->SendEvent(event);
	}
	// frame callback --

	// -- wl keyboard
	InternalScope void wlKeyboardEnter
	(
		void *data,
		wl_keyboard *wlKeyboard,
		uint32_t serial,
		wl_surface *wlSurface,
		wl_array *keys
	)
	{
		(void)data;
		(void)wlKeyboard;
		(void)serial;
		(void)wlSurface;

		/* CIN_TRACE("wl-keyboard: Received keyboard focus"); */

		uint32_t *key;
		wl_array_for_each_casted(key, keys, uint32_t*)
		{
			/* Manage properly? */
		}
	}

	InternalScope void wlKeyboardKey
	(
		void *data,
		wl_keyboard *wlKeyboard,
		uint32_t serial,
		uint32_t time,
		uint32_t key,
		uint32_t keyState
	)
	{
		(void)wlKeyboard;
		(void)serial;
		(void)time;

		Window* window = reinterpret_cast<Window*>(data);

		if(keyState == WL_KEYBOARD_KEY_STATE_PRESSED)
		{
			KeyPressedEvent event(key);
			window->SendEvent(event);
		}
		else
		{
			KeyReleasedEvent event(key);
			window->SendEvent(event);
		}
	}

	InternalScope void wlKeyboardKeymap
	(
		void *data,
		wl_keyboard *wlKeyboard,
		uint32_t format,
		int32_t fd,
		uint32_t size
	)
	{
		(void)data;
		(void)wlKeyboard;
		(void)format;
		(void)size;

		close(fd);
	}

	InternalScope void wlKeyboardLeave
	(
		void *data,
		wl_keyboard *wlKeyboard,
	    uint32_t serial,
		wl_surface *wlSurface
	)
	{
		(void)data;
		(void)wlKeyboard;
		(void)serial;
		(void)wlSurface;

		/* CIN_TRACE("wl-keyboard: Lost keyboard focus"); */
	}

	InternalScope void wlKeyboardModifiers
	(
		void *data,
		wl_keyboard *wlKeyboard,
	    uint32_t serial,
		uint32_t modsDepressed,
	    uint32_t modsLatched,
		uint32_t modsLocked,
	    uint32_t group
	)
	{
		(void)data;
		(void)wlKeyboard;
		(void)serial;
		(void)modsDepressed;
		(void)modsLatched;
		(void)modsLocked;
		(void)group;
	}

	InternalScope void wlKeyboardRepeatInfo
	(
		void *data,
		wl_keyboard *wlKeyboard,
	    int32_t rate,
		int32_t delay
	)
	{
		(void)data;
		(void)wlKeyboard;
	    (void)rate;
		(void)delay;

	    /* Todo: Implement */
	}

	InternalScope constexpr wl_keyboard_listener wlKeyboardListener
	{
		.keymap = wlKeyboardKeymap,
		.enter = wlKeyboardEnter,
		.leave = wlKeyboardLeave,
		.key = wlKeyboardKey,
		.modifiers = wlKeyboardModifiers,
		.repeat_info = wlKeyboardRepeatInfo
	};
	// wl keyboard --

	// -- wl seat
	InternalScope void wlSeatCapabilities
	(
		void *data,
		wl_seat *wlSeat,
		uint32_t capabilities
	)
	{
		(void)data;
		(void)wlSeat;
		(void)capabilities;

		Window* window = reinterpret_cast<Window*>(data);
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

		bool keyboardCapability { bool(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) };

		if(keyboardCapability && windowState->wlKeyboard == NULL)
		{
			windowState->wlKeyboard = wl_seat_get_keyboard(windowState->wlSeat);
			wl_keyboard_add_listener(windowState->wlKeyboard, &wlKeyboardListener, window);
		}
		else if(!keyboardCapability && windowState->wlKeyboard != NULL)
		{
			wl_keyboard_release(windowState->wlKeyboard);
			windowState->wlKeyboard = nullptr;
		}
	}

	InternalScope void wlSeatName
	(
		void *data,
		wl_seat *wlSeat,
		const char *name
	)
	{
		(void)data;
		(void)wlSeat;
		(void)name;
	}

	InternalScope constexpr wl_seat_listener wlSeatListener
	{
		.capabilities = wlSeatCapabilities,
		.name = wlSeatName
	};
	// wl seat --

	// -- xdg shell
	InternalScope void xdgShellPingHandler
	(
	    void *data,
	    zxdg_shell_v6 *xdgShell,
	    uint32_t serial
	)
	{
	    (void)data;

	    zxdg_shell_v6_pong(xdgShell, serial);
	}

	InternalScope constexpr zxdg_shell_v6_listener xdgShellListener
	{
	    .ping = xdgShellPingHandler
	};
	// xdg shell --

	// -- xdg surface
	InternalScope void xdgSurfaceConfigureHandler
	(
	    void *data,
	    zxdg_surface_v6 *xdgSurface,
	    uint32_t serial
	)
	{
	    zxdg_surface_v6_ack_configure(xdgSurface, serial);

	    Window* window = reinterpret_cast<Window*>(data);
		WindowProperties& windowProperties { window->GetProperties() };

		windowProperties.Mode = pending.mode;
		windowProperties.focused = pending.focused;

		if(windowProperties.Width != pending.width || windowProperties.Height != pending.height)
		{
			windowProperties.Width = pending.width;
			windowProperties.Height = pending.height;

			WindowResizedEvent event(window, windowProperties.Width, windowProperties.Height);
			window->SendEvent(event);
		}
	}

	InternalScope  zxdg_surface_v6_listener xdgSurfaceListener
	{
	    .configure {
		[]
		(
			void *data,
			zxdg_surface_v6 *xdgSurface,
			uint32_t serial
		)
		{
			zxdg_surface_v6_ack_configure(xdgSurface, serial);

			Window* window = reinterpret_cast<Window*>(data);
			WindowProperties& windowProperties { window->GetProperties() };

			windowProperties.Mode = pending.mode;
			windowProperties.focused = pending.focused;

			if(windowProperties.Width != pending.width || windowProperties.Height != pending.height)
			{
				CIN_INFO("The desired window dimensions are ({0}, {1}), but the compositor requested a window of size ({2}, {3})",
				windowProperties.Width,
				windowProperties.Height,
				pending.width,
				pending.height);

				windowProperties.Width = pending.width;
				windowProperties.Height = pending.height;

				/* Don't call WindowResizedEvent before Vulkan is ready */
			}
		} }
	};
	// xdg surface --

	// -- xdg toplevel
	InternalScope void xdgToplevelConfigureHandler
	(
	    void *data,
	    zxdg_toplevel_v6 *xdgToplevel,
	    int32_t width,
	    int32_t height,
	    wl_array *states
	)
	{
	    (void)xdgToplevel;

	    Window* window = reinterpret_cast<Window*>(data);
		WindowProperties& windowProperties { window->GetProperties() };

		pending.mode = EWindowMode::Windowed;
		pending.focused = false;

	    const zxdg_toplevel_v6_state* state;
	    wl_array_for_each_casted(state, states, zxdg_toplevel_v6_state*) // c++ being a crybaby about assigning void* to zxdg_toplevel_v6_state*
	    {
	        switch (*state)
	        {
	        case ZXDG_TOPLEVEL_V6_STATE_ACTIVATED:
				pending.focused = false;
	            break;

	        case ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN:
				pending.mode = EWindowMode::Fullscreen;
	            break;

	        case ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED:
				pending.mode = EWindowMode::Maximized;
	            break;

	        case ZXDG_TOPLEVEL_V6_STATE_RESIZING:
	            break;

	        default:
	            break;
	        }
	    }

	    if(width && height)
	    {
			pending.width = width;
			pending.height = height;
	    }
		else
		{
			pending.width = windowProperties.Width;
			pending.height = windowProperties.Height;
		}
	}

	InternalScope void xdgToplevelCloseHandler
	(
	    void *data,
	    zxdg_toplevel_v6 *xdgToplevel
	)
	{
	    (void)xdgToplevel;

	    CIN_TRACE("xdg-shell: Received close event");

	    Window* window = reinterpret_cast<Window*>(data);

	    WindowClosedEvent event(window);
	    window->SendEvent(event);
	}

	InternalScope constexpr zxdg_toplevel_v6_listener xdgToplevelListener
	{
	    .configure = xdgToplevelConfigureHandler,
	    .close = xdgToplevelCloseHandler
	};
	// xdg toplevel --

	// -- registry
	InternalScope void RegistryGlobalHandler
	(
	    void *data,
	    wl_registry *registry,
	    uint32_t name,
	    const char *interface,
	    uint32_t version
	)
	{
	    if (strcmp(interface, wl_compositor_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->wlCompositor =
	        (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);

	        if(wl_compositor_interface.version != int32_t(version))
	            CIN_INFO("using wl_compositor_interface version {0} but the desired version is {1}", version, wl_compositor_interface.version);
	    }
	    else if(strcmp(interface, zxdg_shell_v6_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->xdgShell =
	        (zxdg_shell_v6*)wl_registry_bind(registry, name, &zxdg_shell_v6_interface, version);

	        if(zxdg_shell_v6_interface.version != int32_t(version))
	            CIN_INFO("using zxdg_shell_v6_interface version {0} but the desired version is {1}", version, zxdg_shell_v6_interface.version);
	    }
	    else if(strcmp(interface, wl_output_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->wlOutput =
	        (wl_output*)wl_registry_bind(registry, name, &wl_output_interface, version);

	        if(wl_output_interface.version != int32_t(version))
	            CIN_INFO("using wl_output_interface version {0} but the desired version is {1}", version, wl_output_interface.version);
	    }
	    else if(strcmp(interface, wl_seat_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->wlSeat =
	        (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, version);

	        if(wl_seat_interface.version != int32_t(version))
	            CIN_INFO("using wl_seat_interface version {0} but the desired version is {1}", version, wl_seat_interface.version);
	    }
	}

	InternalScope void RegistryGlobalRemoveHandler
	(
	    void *data,
	    wl_registry *registry,
	    uint32_t name
	)
	{
	    (void)data;
	    (void)registry;
	    (void)name;
	}

	InternalScope constexpr wl_registry_listener registryListener
	{
	    .global = RegistryGlobalHandler,
	    .global_remove = RegistryGlobalRemoveHandler
	};
	// registry --

	InternalScope CIN_FORCE_INLINE void InputSetup(Window* window)
	{
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());
		wl_seat_add_listener(windowState->wlSeat, &wlSeatListener, window);
	}

	InternalScope CIN_FORCE_INLINE void SurfaceSetup(Window* window)
	{
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

	    if(!windowState->xdgShell)
		{
			CIN_CRITICAL("Couldn't find xdg-shell resource, make sure your compositor supports xdg-shell extension");
			CIN_PANIC_EXIT();
		}

        windowState->wlSurface = wl_compositor_create_surface(windowState->wlCompositor);
        wl_surface_add_listener(windowState->wlSurface, &wlSurfaceListener, window);

		wl_callback* fCallback = wl_surface_frame(windowState->wlSurface);
		wl_callback_add_listener(fCallback, &wlSurfaceFrameListener, window);

        windowState->xdgSurface = zxdg_shell_v6_get_xdg_surface(windowState->xdgShell, windowState->wlSurface);
        windowState->xdgToplevel = zxdg_surface_v6_get_toplevel(windowState->xdgSurface);

        zxdg_shell_v6_add_listener(windowState->xdgShell, &xdgShellListener, NULL);
        zxdg_surface_v6_add_listener(windowState->xdgSurface, &xdgSurfaceListener, window);
        zxdg_toplevel_v6_add_listener(windowState->xdgToplevel, &xdgToplevelListener, window);

        wl_surface_commit(windowState->wlSurface);
	}

	InternalScope CIN_FORCE_INLINE void WaylandSetup(Window* window)
	{
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

	    windowState->wlDisplay = wl_display_connect(NULL);
	    if(!windowState->wlDisplay)
		{
			CIN_CRITICAL("Couldn't connect to a wayland display");
			CIN_PANIC_EXIT();
		}

		/* Registry */
	    windowState->wlRegistry = wl_display_get_registry(windowState->wlDisplay);
	    wl_registry_add_listener(windowState->wlRegistry, &registryListener, windowState);
	    wl_display_roundtrip(windowState->wlDisplay);

		InputSetup(window);
		SurfaceSetup(window);

        wl_display_roundtrip(windowState->wlDisplay);
	}

	Window::Window(WindowProperties&& windowProperties, const EventCallbackFunction callback) noexcept
		:
		m_Properties(std::move(windowProperties)),
        m_EventCallback(callback)
	{
        m_State = cinew PlatformWindowState;

		if(m_Properties.Mode != EWindowMode::Unspecified)
			CIN_INFO("The compositor is going to decide on our window's mode");

        WaylandSetup(this);

		SetName(m_Properties.Name);
    }

	Window::~Window() noexcept
	{
        cindel m_State;
    }

	void Window::PollEvents() 
    {
        /* Todo: Handle the dispatching properly */
        wl_display_dispatch_pending(m_State->wlDisplay);

		usleep(10);
	}

	void Window::SendEvent(Event& event)
    {
		CIN_ASSERT(m_EventCallback, "Invalid event callback function");
        m_EventCallback(event);
    }

	const char* Window::GetName() const
    {
        return m_Properties.Name;
    }

	uint32_t Window::GetWidth() const
    {
        return m_Properties.Width;
    }

	uint32_t Window::GetHeight() const
    {
        return m_Properties.Height;
    }

	EWindowMode Window::GetWindowMode() const
    {
        return m_Properties.Mode;
    }

	std::pair<uint32_t, uint32_t> Window::GetSize() const
    {
        return { m_Properties.Width, m_Properties.Height };
    }
    
	const PlatformWindowState* Window::GetState() const 
    {
        return m_State;
    }

	WindowProperties& Window::GetProperties()
	{
		return m_Properties;
	}

	const void* Window::GetNativeHandle() const
    {
		CIN_UNIMPLEMENTED(); return nullptr;
    }

	void Window::SetName(const char* windowName)
    {
        m_Properties.Name = windowName;
        zxdg_toplevel_v6_set_title(m_State->xdgToplevel, m_Properties.Name);
    }

	void Window::SetWidth(const uint32_t windowWidth)
    {
		if(m_Properties.Mode != EWindowMode::Windowed)
			return;

		if(m_Properties.Width == windowWidth)
			return;

		m_Properties.Width = windowWidth;

		WindowResizedEvent event(this, m_Properties.Width, m_Properties.Height);
		SendEvent(event);
    }

	void Window::SetHeight(const uint32_t windowHeight)
    {
		if(m_Properties.Mode != EWindowMode::Windowed)
			return;

		if(m_Properties.Height == windowHeight)
			return;

		m_Properties.Height = windowHeight;

	    WindowResizedEvent event(this, m_Properties.Width, m_Properties.Height);
	    SendEvent(event);
    }

	void Window::SetWindowMode(const EWindowMode windowMode)
    {
		switch(windowMode)
		{
			case EWindowMode::Fullscreen:
			{
				if(m_Properties.Mode != EWindowMode::Fullscreen)
				{
					zxdg_toplevel_v6_set_fullscreen(m_State->xdgToplevel, m_State->wlOutput);
					m_Properties.Mode = EWindowMode::Fullscreen;
				}

				break;
			}

			case EWindowMode::Maximized:
			{
				if(m_Properties.Mode != EWindowMode::Maximized)
				{
					zxdg_toplevel_v6_set_maximized(m_State->xdgToplevel);
					m_Properties.Mode = EWindowMode::Maximized;
				}

				break;
			}

			case EWindowMode::Windowed:
			{
				if(m_Properties.Mode != EWindowMode::Windowed)
				{
					if(m_Properties.Mode == EWindowMode::Fullscreen)
						zxdg_toplevel_v6_unset_fullscreen(m_State->xdgToplevel);

					else if(m_Properties.Mode == EWindowMode::Maximized)
						zxdg_toplevel_v6_unset_maximized(m_State->xdgToplevel);

					m_Properties.Mode = EWindowMode::Windowed;
				}

				break;
			}

			default:
				break;
		}
    }

	void Window::SetSize(std::pair<uint32_t, uint32_t> windowSize)
    {
		if(m_Properties.Mode != EWindowMode::Windowed)
			return;

        const auto [ width, height ] { std::pair<uint32_t, uint32_t>(windowSize.first, windowSize.second) };

		if(m_Properties.Width != width || m_Properties.Height != height)
		{
        	m_Properties.Width = width;
        	m_Properties.Height = height;

	    	WindowResizedEvent event(this, width, height);
	    	SendEvent(event);
		}
    }

    void Window::SetEventCallback(const EventCallbackFunction callback)
    {
		xdgSurfaceListener.configure = xdgSurfaceConfigureHandler;
		m_EventCallback = callback;
    }
} // namespace Cinnamon
#endif // #define CIN_PLATFORM_LINUX