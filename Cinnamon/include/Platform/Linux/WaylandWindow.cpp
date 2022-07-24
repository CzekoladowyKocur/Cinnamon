#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Window.h"

#include <wayland-client.h>
extern "C" {
#include "ThirdParty/xdg/xdg-shell-unstable-v6.h"
}

#include <string.h>
#include <syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

bool terminate = false;

wl_buffer *buffer;
unsigned char* data;

uint32_t lastFrame;

InternalScope void FillBuffer();

struct PixelARGB8888 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
};

/* Declared in Window.h */
struct Cinnamon::PlatformWindowState
{
    // Globals
    wl_display* display {nullptr}; 
    wl_compositor* compositor {nullptr};
    wl_registry* registry {nullptr};
    zxdg_shell_v6* shell {nullptr}; 
    wl_output* output {nullptr};
    wl_seat* seat {nullptr};
    wl_shm* shm {nullptr};

    // Objects 
    wl_surface* waylandSurface {nullptr};
    zxdg_surface_v6* xdgSurface {nullptr};
    zxdg_toplevel_v6* xdgToplevel {nullptr};
};

// -- frame callback
InternalScope void FrameCallback(void* data, wl_callback* frameCallback, uint32_t time); // forward declaration

InternalScope constexpr wl_callback_listener wl_surface_frame_listener =
{
    .done = FrameCallback,
};

InternalScope void FrameCallback(void* data, wl_callback* frameCallback, uint32_t time)
{
    printf("%s\n", "new frame");

    wl_callback_destroy(frameCallback);

    Cinnamon::PlatformWindowState* state = reinterpret_cast<Cinnamon::PlatformWindowState*>(data);
    wl_callback* newFrameCallback = wl_surface_frame(state->waylandSurface);
    wl_callback_add_listener(newFrameCallback, &wl_surface_frame_listener, state);

    FillBuffer();

	wl_surface_attach(state->waylandSurface, buffer, 0, 0);
	wl_surface_damage(state->waylandSurface, 0, 0, 512, 512);
	wl_surface_commit(state->waylandSurface);

    lastFrame = time;
}
// frame callback --

// -- xdg toplevel
InternalScope void xdg_toplevel_configure_handler
(
    void *data,
    zxdg_toplevel_v6 *xdg_toplevel,
    int32_t width,
    int32_t height,
    wl_array *states
)
{
    (void)data;
    (void)xdg_toplevel;
    (void)width;
    (void)height;
    (void)states;

    if(!width && !height) return;
}

InternalScope void xdg_toplevel_close_handler
(
    void *data,
    struct zxdg_toplevel_v6 *xdg_toplevel
)
{
    (void)data;
    (void)xdg_toplevel;

    printf("%s\n", "received close event");

    terminate = true;
}

InternalScope constexpr struct zxdg_toplevel_v6_listener xdgToplevelListener =
{
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler
};
// xdg toplevel --

// -- xdg surface
InternalScope void xdg_surface_configure_handler
(
    void *data,
    struct zxdg_surface_v6 *xdg_surface,
    uint32_t serial
)
{
    (void)data;
    zxdg_surface_v6_ack_configure(xdg_surface, serial);
}

InternalScope constexpr struct zxdg_surface_v6_listener xdg_surface_listener =
{
    .configure = xdg_surface_configure_handler
};
// xdg surface --

// -- xdg shell
InternalScope void xdg_shell_ping_handler
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
    .ping = xdg_shell_ping_handler
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
    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->compositor =
        (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);

        if(wl_compositor_interface.version != int(version))
            printf("%s%i%s%i\n", "warning: using wl_compositor_interface version ", version, " but the wanted version is ", wl_compositor_interface.version);
    }
    else if(strcmp(interface, wl_shm_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->shm =
        (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, version);

        if(wl_shm_interface.version != int(version))
            printf("%s%i%s%i\n", "warning: using wl_shm_interface version ", version, " but the wanted version is ", wl_shm_interface.version);
    }
    else if(strcmp(interface, zxdg_shell_v6_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->shell =
        (zxdg_shell_v6*)wl_registry_bind(registry, name, &zxdg_shell_v6_interface, version);

        if(zxdg_shell_v6_interface.version != int(version))
            printf("%s%i%s%i\n", "warning: using zxdg_shell_v6_interface version ", version, " but the wanted version is ", zxdg_shell_v6_interface.version);
    }
    else if(strcmp(interface, wl_output_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->output =
        (wl_output*)wl_registry_bind(registry, name, &wl_output_interface, version);

        if(wl_output_interface.version != int(version))
            printf("%s%i%s%i\n", "warning: using wl_output_interface version ", version, " but the wanted version is ", wl_output_interface.version);
    }
    else if(strcmp(interface, wl_seat_interface.name) == 0)
    {
        reinterpret_cast<Cinnamon::PlatformWindowState*>(data)->seat =
        (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, version);

        if(wl_seat_interface.version != int(version))
            printf("%s%i%s%i\n", "warning: using wl_seat_interface version ", version, " but the wanted version is ", wl_seat_interface.version);
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


InternalScope void FillBuffer()
{
    uint8_t borderWidth = 4;
    int width = 512;
    int height = 512;

    struct PixelARGB8888* px;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            px = (PixelARGB8888*)(data + y * 512 * sizeof(PixelARGB8888) + x * 4);
            
            if(x < borderWidth || y < borderWidth || x > width - 1 - borderWidth || y > height - 1 - borderWidth) {
                (*px).red = 0;
                (*px).green = 0;
                (*px).blue = 0;
                (*px).alpha = 255;
            } else {
                (*px).red = 255;
                (*px).green = 255;
                (*px).blue = 255;
                (*px).alpha = 255;
            }   
        }
    }
}

InternalScope void WaylandSetup(Cinnamon::PlatformWindowState* state)
{
    state->display = wl_display_connect(NULL);
    if(!state->display) { printf("%s\n", "failed connecting to display"); exit(EXIT_FAILURE); }

    state->registry = wl_display_get_registry(state->display);

    wl_registry_add_listener(state->registry, &registryListener, state);
    wl_display_roundtrip(state->display);
}

namespace Cinnamon {
	Window::Window(WindowProperties&& windowProperties) noexcept
		:
		m_Properties(std::move(windowProperties))
	{
        m_State = new PlatformWindowState;

        WaylandSetup(m_State);
        //m_State->display = wl_display_connect(NULL);
        //m_State->registry = wl_display_get_registry(m_State->display);
  
        //wl_registry_add_listener(m_State->registry, &registryListener, m_State);
        //wl_display_roundtrip(m_State->display);

        //if(!m_State->shell) {
        //    printf("no xdg_shell found in globals - make sure your compositor supports xdg_shell extension");
        //    exit(EXIT_FAILURE);
        //}

        m_State->waylandSurface =  wl_compositor_create_surface(m_State->compositor);
        m_State->xdgSurface = zxdg_shell_v6_get_xdg_surface(m_State->shell, m_State->waylandSurface);
        m_State->xdgToplevel = zxdg_surface_v6_get_toplevel(m_State->xdgSurface);

        wl_surface_commit(m_State->waylandSurface);

        int width = 512;
        int height = 512;
        int stride = width * 4;
        int size = stride * height;  // bytes

        // open an anonymous file and write zeros bytes to it
        int fd = syscall(SYS_memfd_create, "buffer", 0);
        ftruncate(fd, size);

        data = (unsigned char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        struct wl_shm_pool *pool = wl_shm_create_pool(m_State->shm, fd, size);

        buffer = wl_shm_pool_create_buffer(pool,
            0, width, height, stride, WL_SHM_FORMAT_ARGB8888);


        // Waiting for the compositor to configure the surface 
        wl_display_roundtrip(m_State->display);

        zxdg_shell_v6_add_listener(m_State->shell, &xdgShellListener, NULL);
        zxdg_toplevel_v6_add_listener(m_State->xdgToplevel, &xdgToplevelListener, NULL);

        SetName(u8"Hello");

        struct wl_callback* frameCallback = wl_surface_frame(m_State->waylandSurface);
        wl_callback_add_listener(frameCallback, &wl_surface_frame_listener, m_State);

        wl_surface_commit(m_State->waylandSurface);

        while(wl_display_dispatch(m_State->display) && !terminate) { usleep(1); }

        zxdg_toplevel_v6_destroy(m_State->xdgToplevel);
        zxdg_surface_v6_destroy(m_State->xdgSurface);
        wl_surface_destroy(m_State->waylandSurface);
        wl_display_disconnect(m_State->display);

    }

	Window::~Window() noexcept
	{}

    void Window::SetName(const char8_t* windowName)
    {
        m_Properties.Name = windowName;
        zxdg_toplevel_v6_set_title(m_State->xdgToplevel, reinterpret_cast<const char*>(m_Properties.Name));
    }
}
#endif