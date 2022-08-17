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
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>

#define wl_array_for_each_casted(pos, array, type) \
	for \
    (pos = reinterpret_cast<type>((array)->data); \
    (const char *) pos < ((const char *) (array)->data + (array)->size); \
    (pos)++)

/* Todo: Manage closing the window and destroying this object properly */

//InternalScope void WaylandCleanup(Cinnamon::PlatformWindowState* state)
//{
//    if(state->seat)
//        wl_seat_destroy(state->seat);
//
//    if(state->output)
//        wl_output_destroy(state->output);
//
//    if(state->xdgToplevel)
//        zxdg_toplevel_v6_destroy(state->xdgToplevel);
//
//    if(state->xdgSurface)
//        zxdg_surface_v6_destroy(state->xdgSurface);
//
//    if(state->xdgShell)
//        zxdg_shell_v6_destroy(state->xdgShell);
//
//    if(state->waylandSurface)
//        wl_surface_destroy(state->waylandSurface);
//
//    if(state->compositor)
//        wl_compositor_destroy(state->compositor);
//    
//    if(state->registry)
//        wl_registry_destroy(state->registry);
//
//    if(state->display)
//        wl_display_disconnect(state->display);
//}

namespace Cinnamon {

	/* Declared in Window.h */
	struct PlatformWindowState
	{
	    // Globals
	    wl_display* display { nullptr };
	    wl_compositor* compositor { nullptr };
	    wl_registry* registry { nullptr };
	    zxdg_shell_v6* xdgShell { nullptr };
	    wl_output* output { nullptr };
	    wl_seat* wlSeat { nullptr };

	    // Objects

		/* Surfaces*/
	    wl_surface* wlSurface { nullptr };
	    zxdg_surface_v6* xdgSurface { nullptr };
	    zxdg_toplevel_v6* xdgToplevel { nullptr };

		/* Input */
		xkb_context* xkbContext { nullptr };
		xkb_state* xkbState { nullptr };
		xkb_keymap* xkbKeymap { nullptr };
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
		(void)wlKeyboard;

		Window* window = reinterpret_cast<Window*>(data);
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

		CIN_ASSERT(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

		char* map_shm = reinterpret_cast<char*>(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
		CIN_ASSERT(map_shm != MAP_FAILED);

		xkb_keymap *xkbKeymap = xkb_keymap_new_from_string(windowState->xkbContext, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

		munmap(map_shm, size);
		close(fd);

		xkb_state *xkbState = xkb_state_new(xkbKeymap);
		xkb_keymap_unref(windowState->xkbKeymap);
		xkb_state_unref(windowState->xkbState);
		windowState->xkbKeymap = xkbKeymap;
		windowState->xkbState = xkbState;

		CIN_TRACE("wl-keyboard: Got keymap");
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

		CIN_TRACE("wl-seat: Seat name: {0}", name);
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
	    (void)data;
	    zxdg_surface_v6_ack_configure(xdgSurface, serial);
	}

	InternalScope constexpr zxdg_surface_v6_listener xdgSurfaceListener
	{
	    .configure = xdgSurfaceConfigureHandler
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

		windowProperties.Mode = EWindowMode::Windowed;

	    const zxdg_toplevel_v6_state* state;
	    wl_array_for_each_casted(state, states, zxdg_toplevel_v6_state*) // c++ being a crybaby about assigning void* to zxdg_toplevel_v6_state*
	    {
	        switch (*state)
	        {
	        case ZXDG_TOPLEVEL_V6_STATE_ACTIVATED:
	            /* Todo: implement */
	            break;

	        case ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN:
	            CIN_INFO("xdg-shell: Fullscreen");
	            /* window->SetWindowMode(EWindowMode::Fullscreen); */
				windowProperties.Mode = EWindowMode::Fullscreen;
	            break;

	        case ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED:
	            CIN_INFO("xdg-shell: Maximized");
	            /* window->SetWindowMode(EWindowMode::Maximized); */
				windowProperties.Mode = EWindowMode::Maximized;
	            break;

	        case ZXDG_TOPLEVEL_V6_STATE_RESIZING:
				/* window->SetWindowMode(EWindowMode::Windowed); */
				/* windowProperties.Mode = EWindowMode::Windowed; */
	            break;

	        default:
	            break;
	        }
	    }

		/* Todo: Properly manage { 0, 0 } */
	    if(width && height)
	    {
	        const auto [ windowWidth, windowHeight ] { window->GetSize() };
	        const uint32_t uWidth = (uint32_t)width;
	        const uint32_t uHeight = (uint32_t)height;

	        if(uWidth != windowWidth || uHeight != windowHeight)
	        {
	            window->SetSize({ uWidth, uHeight });

	            WindowResizedEvent event(window, uWidth, uHeight);
	            window->SendEvent(event);
	        }
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
	        reinterpret_cast<PlatformWindowState*>(data)->compositor =
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
	        reinterpret_cast<PlatformWindowState*>(data)->output =
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

	InternalScope bool WaylandSetup(PlatformWindowState* state)
	{
	    state->display = wl_display_connect(NULL);
	    if(!state->display)
	        return 0;

	    state->registry = wl_display_get_registry(state->display);
	    if(!state->registry)
	        return 0;

	    wl_registry_add_listener(state->registry, &registryListener, state);

	    wl_display_roundtrip(state->display);

	    if(!state->compositor) { return 0; }
	    if(!state->xdgShell) { CIN_CRITICAL("Couldn't find xdg-shell resource, make sure your compositor supports xdg-shell extension"); return 0; }

	    return 1;
	}

	Window::Window(WindowProperties&& windowProperties, const EventCallbackFunction callback) noexcept
		:
		m_Properties(std::move(windowProperties)),
        m_EventCallback(callback)
	{
        m_State = cinew PlatformWindowState;

        if(!WaylandSetup(m_State))
        {
            CIN_CRITICAL("Failed connecting to the display or retrieving globals from the registry");

            /* Todo: Add wayland cleanup */

			cindel m_State;

            exit(EXIT_FAILURE);
        }

		m_State->xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

		wl_seat_add_listener(m_State->wlSeat, &wlSeatListener, this);

        m_State->wlSurface = wl_compositor_create_surface(m_State->compositor);
        wl_surface_add_listener(m_State->wlSurface, &wlSurfaceListener, this);

		wl_callback* newFrameCallback = wl_surface_frame(m_State->wlSurface);
        wl_callback_add_listener(newFrameCallback, &wlSurfaceFrameListener, this);

        m_State->xdgSurface = zxdg_shell_v6_get_xdg_surface(m_State->xdgShell, m_State->wlSurface);
        m_State->xdgToplevel = zxdg_surface_v6_get_toplevel(m_State->xdgSurface);

        zxdg_shell_v6_add_listener(m_State->xdgShell, &xdgShellListener, NULL);
        zxdg_surface_v6_add_listener(m_State->xdgSurface, &xdgSurfaceListener, this);
        zxdg_toplevel_v6_add_listener(m_State->xdgToplevel, &xdgToplevelListener, this);

        wl_surface_commit(m_State->wlSurface);

        wl_display_roundtrip(m_State->display);

        m_Properties.Mode = EWindowMode::Unspecified;
    }

	Window::~Window() noexcept
	{
        cindel m_State;
    }

	void Window::PollEvents() 
    {
        /* Todo: Handle the dispatching properly */
        wl_display_dispatch_pending(m_State->display);

        usleep(10);
    }

	void Window::SendEvent(Event& event)
    {
		CIN_ASSERT(m_EventCallback, "Invalid event callback function");
        m_EventCallback(event);
    }

	const char8_t* Window::GetName() const
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

	void Window::SetName(const char8_t* windowName)
    {
        m_Properties.Name = windowName;
        zxdg_toplevel_v6_set_title(m_State->xdgToplevel, reinterpret_cast<const char*>(m_Properties.Name));
    }

	void Window::SetWidth(const uint32_t windowWidth)
    {
        CIN_UNIMPLEMENTED(); CIN_UNUSED(windowWidth);
    }

	void Window::SetHeight(const uint32_t windowHeight)
    {
        CIN_UNIMPLEMENTED(); CIN_UNUSED(windowHeight);
    }

	void Window::SetWindowMode(const EWindowMode windowMode)
    {
		switch(windowMode)
		{
			case EWindowMode::Fullscreen:
			{
				if(m_Properties.Mode != EWindowMode::Fullscreen)
				{
					/* m_Properties.Mode = EWindowMode::Fullscreen; */
					zxdg_toplevel_v6_set_fullscreen(m_State->xdgToplevel, m_State->output);
				}

				break;
			}

			case EWindowMode::Maximized:
			{
				if(m_Properties.Mode != EWindowMode::Maximized)
				{
					/* m_Properties.Mode = EWindowMode::Maximized; */
					zxdg_toplevel_v6_set_maximized(m_State->xdgToplevel);
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
				}

				break;
			}

			default:
				break;
		}
    }

	void Window::SetSize(std::pair<uint32_t, uint32_t> windowSize) // Huh
    {
        const auto [ width, height ] { std::pair<uint32_t, uint32_t>(windowSize.first, windowSize.second) };

        m_Properties.Width = width;
        m_Properties.Height = height;
    }

    void Window::SetEventCallback(const EventCallbackFunction callback)
    {
        m_EventCallback = callback;
    }
} // namespace Cinnamon
#endif // #define CIN_PLATFORM_LINUX