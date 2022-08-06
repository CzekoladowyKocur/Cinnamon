#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"

#include <wayland-client.h>
extern "C"
{
#include "ThirdParty/xdg/xdg-shell-unstable-v6.h"
}

/* Todo: Manage closing the window and destroying this object properly */

/* Declared in Window.h */
struct Cinnamon::PlatformWindowState
{
    // Globals
    wl_display* display {nullptr}; 
    wl_compositor* compositor {nullptr};
    wl_registry* registry {nullptr};
    zxdg_shell_v6* xdgShell {nullptr}; 
    wl_output* output {nullptr};
    wl_seat* seat {nullptr};

    // Objects 
    wl_surface* waylandSurface {nullptr};
    zxdg_surface_v6* xdgSurface {nullptr};
    zxdg_toplevel_v6* xdgToplevel {nullptr};
};

// -- frame callback
InternalScope void FrameCallback(void* data, wl_callback* frameCallback, uint32_t time);

InternalScope constexpr wl_callback_listener wl_surface_frame_listener =
{
    .done = FrameCallback,
};

InternalScope void FrameCallback(void* data, wl_callback* frameCallback, uint32_t time)
{
    (void)time;
    printf("%s\n", "new frame");

    wl_callback_destroy(frameCallback);

    Cinnamon::PlatformWindowState* state = reinterpret_cast<Cinnamon::PlatformWindowState*>(data);
    wl_callback* newFrameCallback = wl_surface_frame(state->waylandSurface);
    wl_callback_add_listener(newFrameCallback, &wl_surface_frame_listener, state);

    /* Todo: properly manage frame callback */

    //FillBuffer();

	//wl_surface_attach(state->waylandSurface, buffer, 0, 0);
	//wl_surface_damage(state->waylandSurface, 0, 0, 512, 512);
	//wl_surface_commit(state->waylandSurface);

    //lastFrame = time;
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
    Cinnamon::Window* window = reinterpret_cast<Cinnamon::Window*>(data);

    (void)xdgToplevel;
    (void)states;
    (void)window;
    (void)width;
    (void)height;

    auto windowSize = window->GetSize();
    if((uint32_t)width != windowSize.first || (uint32_t)height != windowSize.second)
    {
        if(!width || !height)
            return;
        printf("%s%i%s%i\n", "w, h: ", width, ", ", height);
        {
            window->SetSize({(uint32_t)width, (uint32_t)height});
            Cinnamon::WindowResizedEvent event(window, width, height);
            window->SendEvent(event);
        }

        {
            Cinnamon::ApplicationRenderEvent event;
            window->SendEvent(event);
        }

    }
}

InternalScope void xdgToplevelCloseHandler
(
    void *data,
    struct zxdg_toplevel_v6 *xdg_toplevel
)
{
    (void)xdg_toplevel;

    printf("%s\n", "XdgToplevelCloseHandler: Received close event.");

    Cinnamon::Window* window = reinterpret_cast<Cinnamon::Window*>(data);

    Cinnamon::WindowClosedEvent event(window);
    window->SendEvent(event);
}

InternalScope constexpr struct zxdg_toplevel_v6_listener xdgToplevelListener =
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

InternalScope constexpr struct zxdg_surface_v6_listener xdgSurfaceListener =
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

InternalScope constexpr struct zxdg_shell_v6_listener xdgShellListener =
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
    //auto ptr = reinterpret_cast<Cinnamon::PlatformWindowState*>(data);
    //if (strcmp(interface, wl_display_interface.name) == 0)
    //    ptr->display = (wl_display*)wl_registry_bind(registry, name, &wl_display_interface, version);

    //if (strcmp(interface, wl_compositor_interface.name) == 0)
    //    ptr->compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);

    //if (strcmp(interface, zxdg_shell_v6_interface.name) == 0)
    //    ptr->xdgShell = (zxdg_shell_v6*)wl_registry_bind(registry, name, &zxdg_shell_v6_interface, version);

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->compositor =
        (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);

        if(wl_compositor_interface.version != int(version))
            printf("%s%i%s%i\n", "info: using wl_compositor_interface version ", version, " but the wanted version is ", wl_compositor_interface.version);
    }
    else if(strcmp(interface, zxdg_shell_v6_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->xdgShell =
        (zxdg_shell_v6*)wl_registry_bind(registry, name, &zxdg_shell_v6_interface, version);

        if(zxdg_shell_v6_interface.version != int(version))
            printf("%s%i%s%i\n", "info: using zxdg_shell_v6_interface version ", version, " but the wanted version is ", zxdg_shell_v6_interface.version);
    }
    else if(strcmp(interface, wl_output_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->output =
        (wl_output*)wl_registry_bind(registry, name, &wl_output_interface, version);

        if(wl_output_interface.version != int(version))
            printf("%s%i%s%i\n", "info: using wl_output_interface version ", version, " but the wanted version is ", wl_output_interface.version);
    }
    else if(strcmp(interface, wl_seat_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->seat =
        (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, version);

        if(wl_seat_interface.version != int(version))
            printf("%s%i%s%i\n", "info: using wl_seat_interface version ", version, " but the wanted version is ", wl_seat_interface.version);
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

InternalScope constexpr struct wl_registry_listener registryListener =
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
    if(!state->xdgShell) { printf("%s%i\n", "Could not find xdg-shell resource. Make sure your compositor supports xdg-shell interface version ", zxdg_shell_v6_interface.version); return 0; }

    return 1;
}

namespace Cinnamon {

	Window::Window(WindowProperties&& windowProperties, const EventCallbackFunction callback) noexcept
		:
		m_Properties(std::move(windowProperties)),
        m_EventCallback(callback)
	{
        m_State = new PlatformWindowState;

        printf("%s%i%s%i\n", "width, height: ", m_Properties.Width, ", ", m_Properties.Height);

        if(!WaylandSetup(m_State))
        {
            printf("%s\n", "Failed connecting to the display or retrieving globals from registry");
            /* Todo: Add wayland cleanup */
            exit(EXIT_FAILURE);
        }

        m_State->waylandSurface = wl_compositor_create_surface(m_State->compositor);
        m_State->xdgSurface = zxdg_shell_v6_get_xdg_surface(m_State->xdgShell, m_State->waylandSurface);
        m_State->xdgToplevel = zxdg_surface_v6_get_toplevel(m_State->xdgSurface);

        zxdg_shell_v6_add_listener(m_State->xdgShell, &xdgShellListener, NULL);
        zxdg_toplevel_v6_add_listener(m_State->xdgToplevel, &xdgToplevelListener, this);

        zxdg_toplevel_v6_set_max_size(m_State->xdgToplevel, 2560, 1440);

        wl_surface_commit(m_State->waylandSurface);
        wl_display_roundtrip(m_State->display);
    }

	Window::~Window() noexcept
	{
        delete m_State;
    }

	void Window::PollEvents()
    {
        /* Todo: Handle the dispatching properly */
		ApplicationRenderEvent event;
	    SendEvent(event);

        wl_display_dispatch_pending(m_State->display);
    }

	void Window::SendEvent(Event& event)
    {		
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
        return { m_Properties.Width, m_Properties.Width };
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
        CIN_UNIMPLEMENTED(); CIN_UNUSED(windowMode);
    }

	void Window::SetSize(std::pair<uint32_t, uint32_t> windowSize)
    {
        m_Properties.Width = windowSize.first;
        m_Properties.Height = windowSize.second;
    }
} // namespace Cinnamon
#endif // #define CIN_PLATFORM_LINUX