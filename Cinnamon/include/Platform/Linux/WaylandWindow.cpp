#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"

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
/* Restructure everything to work cleanly with namespace & window class */

/* Declared in Window.h */
struct Cinnamon::PlatformWindowState
{
    // Globals
    wl_display* display { nullptr };
    wl_compositor* compositor { nullptr };
    wl_registry* registry { nullptr };
    zxdg_shell_v6* xdgShell { nullptr };
    wl_output* output { nullptr };
    wl_seat* seat { nullptr };

    // Objects 
    wl_surface* waylandSurface { nullptr };
    zxdg_surface_v6* xdgSurface { nullptr };
    zxdg_toplevel_v6* xdgToplevel { nullptr };
};

// -- wayland surface
InternalScope void WaylandSurfaceEnter
(
    void* data,
    wl_surface* waylandSurface,
    wl_output* output
)
{
    /* Todo: implement */
    (void)data;
    (void)waylandSurface;
    (void)output;
}

InternalScope void WaylandSurfaceLeave
(
    void* data,
    wl_surface* waylandSurface,
    wl_output* output
)
{
    /* Todo: implement */
    (void)data;
    (void)waylandSurface;
    (void)output;
}

InternalScope constexpr wl_surface_listener waylandSurfaceListener
{
    .enter = WaylandSurfaceEnter,
    .leave = WaylandSurfaceLeave
};
// wayland surface --

// -- frame callback
InternalScope bool paint { false };

InternalScope void FrameCallback(void* data, wl_callback* frameCallback, uint32_t time);

InternalScope constexpr wl_callback_listener wl_surface_frame_listener
{
    .done = FrameCallback
};

InternalScope void FrameCallback(void* data, wl_callback* frameCallback, uint32_t time)
{
    (void)time;
    /* Todo: properly manage frame callback */

    wl_callback_destroy(frameCallback);

    Cinnamon::PlatformWindowState* state = reinterpret_cast<Cinnamon::PlatformWindowState*>(data);
    wl_callback* newFrameCallback = wl_surface_frame(state->waylandSurface);
    wl_callback_add_listener(newFrameCallback, &wl_surface_frame_listener, state);

    paint = true;
}
// frame callback --

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

    Cinnamon::Window* window = reinterpret_cast<Cinnamon::Window*>(data);

    const zxdg_toplevel_v6_state* state;
    wl_array_for_each_casted(state, states, zxdg_toplevel_v6_state*) // c++ being a crybaby about assigning void* to zxdg_toplevel_v6_state*
    {
        switch (*state)
        {
        case ZXDG_TOPLEVEL_V6_STATE_ACTIVATED:
            /* Todo */
            break;

        case ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN:
            using namespace Cinnamon;
            CIN_INFO("xdg-shell: Fullscreen");
            window->SetWindowMode(Cinnamon::EWindowMode::WindowedFullscreen);
            break;

        case ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED:
            using namespace Cinnamon;
            CIN_INFO("xdg-shell: Maximized");
            window->SetWindowMode(Cinnamon::EWindowMode::Maximized);
            break;

        case ZXDG_TOPLEVEL_V6_STATE_RESIZING:
            window->SetWindowMode(Cinnamon::EWindowMode::Windowed);
            break;

        default:
            using namespace Cinnamon;
            CIN_WARN("should not happenTM at {0}", __FILE__);
            break;
        }
    }

    if(width && height)
    {
        const auto [ windowWidth, windowHeight ] { window->GetSize() };
        const uint32_t uWidth = (uint32_t)width;
        const uint32_t uHeight = (uint32_t)height;

        if(uWidth != windowWidth || uHeight != windowHeight)
        {
            window->SetSize({ uWidth, uHeight });

            Cinnamon::WindowResizedEvent event(window, uWidth, uHeight);
            window->SendEvent(event);
        }
    }
}

InternalScope void xdgToplevelCloseHandler
(
    void *data,
    struct zxdg_toplevel_v6 *xdgToplevel
)
{
    (void)xdgToplevel;

    using namespace Cinnamon;
    CIN_TRACE("xdg-shell: Received close event");

    Cinnamon::Window* window = reinterpret_cast<Cinnamon::Window*>(data);

    Cinnamon::WindowClosedEvent event(window);
    window->SendEvent(event);
}

InternalScope constexpr struct zxdg_toplevel_v6_listener xdgToplevelListener
{
    .configure = xdgToplevelConfigureHandler,
    .close = xdgToplevelCloseHandler
};
// xdg toplevel --

// -- xdg surface
InternalScope void xdgSurfaceConfigureHandler
(
    void *data,
    struct zxdg_surface_v6 *xdg_surface,
    uint32_t serial
)
{
    (void)data;
    zxdg_surface_v6_ack_configure(xdg_surface, serial);
}

InternalScope constexpr struct zxdg_surface_v6_listener xdgSurfaceListener
{
    .configure = xdgSurfaceConfigureHandler
};
// xdg surface --

// -- xdg shell
InternalScope void xdgShellPingHandler
(
    void *data,
    struct zxdg_shell_v6 *xdg_shell,
    uint32_t serial
)
{
    (void)data;

    zxdg_shell_v6_pong(xdg_shell, serial);
}

InternalScope constexpr struct zxdg_shell_v6_listener xdgShellListener
{
    .ping = xdgShellPingHandler
};
// xdg shell --

// -- registry
InternalScope void RegistryGlobalHandler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name,
    const char *interface,
    uint32_t version
) 
{
    using namespace Cinnamon;

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->compositor =
        (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);

        if(wl_compositor_interface.version != int32_t(version))
            CIN_INFO("using wl_compositor_interface version {0} but the desired version is {1}", version, wl_compositor_interface.version);
    }
    else if(strcmp(interface, zxdg_shell_v6_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->xdgShell =
        (zxdg_shell_v6*)wl_registry_bind(registry, name, &zxdg_shell_v6_interface, version);

        if(zxdg_shell_v6_interface.version != int32_t(version))
            CIN_INFO("using zxdg_shell_v6_interface version {0} but the desired version is {1}", version, zxdg_shell_v6_interface.version);
    }
    else if(strcmp(interface, wl_output_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->output =
        (wl_output*)wl_registry_bind(registry, name, &wl_output_interface, version);

        if(wl_output_interface.version != int32_t(version))
            CIN_INFO("using wl_output_interface version {0} but the desired version is {1}", version, wl_output_interface.version);
    }
    else if(strcmp(interface, wl_seat_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->seat =
        (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, version);

        if(wl_seat_interface.version != int32_t(version))
            CIN_INFO("using wl_seat_interface version {0} but the desired version is {1}", version, wl_seat_interface.version);

    }
}

InternalScope void RegistryGlobalRemoveHandler
(
    void *data,
    struct wl_registry *registry,
    uint32_t name
) 
{
    (void)data;
    (void)registry;
    (void)name;
}

InternalScope constexpr struct wl_registry_listener registryListener
{
    .global = RegistryGlobalHandler,
    .global_remove = RegistryGlobalRemoveHandler 
};
// registry --

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

InternalScope bool WaylandSetup(Cinnamon::PlatformWindowState* state)
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
    if(!state->xdgShell) { using namespace Cinnamon; CIN_CRITICAL("Couldn't find xdg-shell resource, make sure your compositor supports xdg-shell extension"); return 0; }

    return 1;
}

namespace Cinnamon {

	Window::Window(WindowProperties&& windowProperties, const EventCallbackFunction callback) noexcept
		:
		m_Properties(std::move(windowProperties)),
        m_EventCallback(callback)
	{
        //m_State = new PlatformWindowState;
        m_State = cinew PlatformWindowState;

        if(!WaylandSetup(m_State))
        {
            CIN_CRITICAL("Failed connecting to the display or retrieving globals from the registry");
            /* Todo: Add wayland cleanup */
            exit(EXIT_FAILURE);
        }

        m_State->waylandSurface = wl_compositor_create_surface(m_State->compositor);
        wl_surface_add_listener(m_State->waylandSurface, &waylandSurfaceListener, this);

        m_State->xdgSurface = zxdg_shell_v6_get_xdg_surface(m_State->xdgShell, m_State->waylandSurface);
        m_State->xdgToplevel = zxdg_surface_v6_get_toplevel(m_State->xdgSurface);

        zxdg_shell_v6_add_listener(m_State->xdgShell, &xdgShellListener, NULL);
        zxdg_surface_v6_add_listener(m_State->xdgSurface, &xdgSurfaceListener, this);
        zxdg_toplevel_v6_add_listener(m_State->xdgToplevel, &xdgToplevelListener, this);

        wl_callback* newFrameCallback = wl_surface_frame(m_State->waylandSurface);
        wl_callback_add_listener(newFrameCallback, &wl_surface_frame_listener, m_State);

        wl_surface_commit(m_State->waylandSurface);
        wl_display_roundtrip(m_State->display);

        m_Properties.Mode = EWindowMode::Windowed;
    }

	Window::~Window() noexcept
	{
        cindel m_State;
    }

	void Window::PollEvents()
    {
        /* Todo: Handle the dispatching properly */
        wl_display_dispatch_pending(m_State->display);

        if(paint) {
            paint = false;

            ApplicationRenderEvent event;
            SendEvent(event);
        }

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
    
	const Cinnamon::PlatformWindowState* Window::GetState() const 
    {
        return m_State;
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
        m_Properties.Mode = windowMode;
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