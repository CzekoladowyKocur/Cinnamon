#include "Cinnamon/include/Core/Logger.h"
#include <cstdint>
#include <cstdio>
#include <wayland-client-protocol.h>
#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/TypeDefines.h"
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Core/Input.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"
#include "Cinnamon/include/Event/MouseEvent.h"
#include "Cinnamon/include/Event/KeyEvent.h"

#include <wayland-client.h>
extern "C"
{
#include "ThirdParty/xdg/xdg-shell.h"
}

#include <sys/poll.h>

#define wl_array_for_each_casted(pos, array, type) \
	for \
	(pos = reinterpret_cast<type>((array)->data); \
	(const char *) pos < ((const char *) (array)->data + (array)->size); \
	(pos)++)

/* Todo: Manage closing the window and destroying this object properly */

// Temp 
#include "ThirdParty/imgui/imgui.h"
using namespace Cinnamon;

ImGuiKey NativeKeyCodeToImGUIKeyCode(const KeyCode native)
{
	switch(native)
	{
		// Taken from imgui.h
    	case int(Key::Tab): return ImGuiKey_Tab;             // == ImGuiKey_NamedKey_BEGIN
    	case int(Key::Left): return ImGuiKey_LeftArrow;
    	case int(Key::Right): return ImGuiKey_RightArrow;
    	case int(Key::Up): return ImGuiKey_UpArrow;
    	case int(Key::Down): return ImGuiKey_DownArrow;
    	//case int(Key::PageUp): ImGuiKey_PageUp;
    	//ImGuiKey_PageDown;
    	case int(Key::Home): return ImGuiKey_Home;
    	case int(Key::End): return ImGuiKey_End;
    	case int(Key::Insert): return ImGuiKey_Insert;
    	case int(Key::Delete): return ImGuiKey_Delete;
    	case int(Key::Backspace): return ImGuiKey_Backspace;
    	case int(Key::Space): return ImGuiKey_Space;
    	case int(Key::Enter): return ImGuiKey_Enter;
    	case int(Key::Escape): return ImGuiKey_Escape;
    	case int(Key::LeftControl): return ImGuiKey_LeftCtrl; 
		case int(Key::LeftShift): return ImGuiKey_LeftShift; 
		//case int(Key::LeftAlt): return ImGuiKey_LeftAlt; 
		//ImGuiKey_LeftSuper;
    	case int(Key::RightControl): return ImGuiKey_RightCtrl; 
		case int(Key::RightShift): return ImGuiKey_RightShift; 
		//ImGuiKey_RightAlt; 
		//ImGuiKey_RightSuper;
    	//case int(Key::Menu): return ImGuiKey_Menu;
    	//case int(Key::Number0): return ImGuiKey_0;
		//ImGuiKey_1; 
		//ImGuiKey_2; 
		//ImGuiKey_3;
		//ImGuiKey_4; 
		//ImGuiKey_5;
		//ImGuiKey_6; 
		//ImGuiKey_7; 
		//ImGuiKey_8; 
		//ImGuiKey_9;
    	case int(Key::A): return ImGuiKey_A; 
		case int(Key::B): return ImGuiKey_B; 
		case int(Key::C): return ImGuiKey_C; 
		case int(Key::D): return ImGuiKey_D; 
		case int(Key::E): return ImGuiKey_E;
		case int(Key::F): return ImGuiKey_F; 
		case int(Key::G): return ImGuiKey_G; 
		case int(Key::H): return ImGuiKey_H; 
		case int(Key::I): return ImGuiKey_I; 
		case int(Key::J): return ImGuiKey_J;
    	case int(Key::K): return ImGuiKey_K; 
		case int(Key::L): return ImGuiKey_L; 
		case int(Key::M): return ImGuiKey_M; 
		case int(Key::N): return ImGuiKey_N; 
		case int(Key::O): return ImGuiKey_O; 
		case int(Key::P): return ImGuiKey_P; 
		case int(Key::Q): return ImGuiKey_Q; 
		case int(Key::R): return ImGuiKey_R; 
		case int(Key::S): return ImGuiKey_S; 
		case int(Key::T): return ImGuiKey_T;
    	case int(Key::U): return ImGuiKey_U; 
		case int(Key::V): return ImGuiKey_V; 
		case int(Key::W): return ImGuiKey_W; 
		case int(Key::X): return ImGuiKey_X; 
		case int(Key::Y): return ImGuiKey_Y;
		case int(Key::Z): return ImGuiKey_Z;
		/* ImGuiKey_F1;
		ImGuiKey_F2;
		ImGuiKey_F3;
		ImGuiKey_F4;
		ImGuiKey_F5;
		ImGuiKey_F6;
		ImGuiKey_F7;
		ImGuiKey_F8;
		ImGuiKey_F9;
		ImGuiKey_F10;
		ImGuiKey_F11;
		ImGuiKey_F12;
		ImGuiKey_Apostrophe;        // '
		ImGuiKey_Comma;             // ;
		ImGuiKey_Minus;             // -
		ImGuiKey_Period;            // .
		ImGuiKey_Slash;             // /
		ImGuiKey_Semicolon;         // ;
		ImGuiKey_Equal;             // =
		ImGuiKey_LeftBracket;       // [
		ImGuiKey_Backslash;         // \ (this text inhibit multiline comment caused by backslash)
		ImGuiKey_RightBracket;      // ]
		ImGuiKey_GraveAccent;       // `
		ImGuiKey_CapsLock;
		ImGuiKey_ScrollLock;
		ImGuiKey_NumLock;
		ImGuiKey_PrintScreen;
		ImGuiKey_Pause;
		ImGuiKey_Keypad0; ImGuiKey_Keypad1; ImGuiKey_Keypad2; ImGuiKey_Keypad3; ImGuiKey_Keypad4;
		ImGuiKey_Keypad5; ImGuiKey_Keypad6; ImGuiKey_Keypad7; ImGuiKey_Keypad8; ImGuiKey_Keypad9;
		ImGuiKey_KeypadDecimal;
		ImGuiKey_KeypadDivide;
		ImGuiKey_KeypadMultiply;
		ImGuiKey_KeypadSubtract;
		ImGuiKey_KeypadAdd;
		ImGuiKey_KeypadEnter;
		ImGuiKey_KeypadEqual; */
		default: return ImGuiKey_None;
	}

	CIN_ASSERT(false);
	return ImGuiKey_None;
}

static void AddKeyEventGUI(const KeyCode native, const bool pressed)
{
	const ImGuiKey key{ NativeKeyCodeToImGUIKeyCode(native) };

	ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(key, pressed);
}

static ImGuiMouseButton NativeMouseButtonCodeToImGuiMouseButtonCode(const MouseCode native)
{
	switch(native)
	{
		case int(Mouse::LeftButton): 	return ImGuiMouseButton_Left;
		case int(Mouse::MiddleButton): 	return ImGuiMouseButton_Middle;
		case int(Mouse::RightButton): 	return ImGuiMouseButton_Right;
		// TODO: Handle it better 
		default: break;
	}

	return ImGuiMouseButton_Right;
}

static void AddMouseButtonEventGUI(const MouseCode native, const bool pressed)
{
	const ImGuiMouseButton mouseButton{ NativeMouseButtonCodeToImGuiMouseButtonCode(native) };

	ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(mouseButton, pressed);	
}

static void AddMousePositionEventGUI(const float xPosition, const float yPosition)
{
	ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(xPosition, yPosition);	
}

namespace Cinnamon {

	/* https://wayland-book.com/seat/example.html */
	enum EPointerEventMask
	{
		POINTER_EVENT_ENTER = 1 << 0,
		POINTER_EVENT_LEAVE = 1 << 1,
		POINTER_EVENT_MOTION = 1 << 2,
		POINTER_EVENT_BUTTON = 1 << 3,
		POINTER_EVENT_AXIS = 1 << 4,
		POINTER_EVENT_AXIS_SOURCE = 1 << 5,
		POINTER_EVENT_AXIS_STOP = 1 << 6,
		POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
	};

	struct PointerEvent
	{
		uint32_t eventMask;
		wl_fixed_t x, y;
		uint32_t button, state;
		uint32_t serial;
		struct {
			bool valid;
			wl_fixed_t value;
			int32_t discrete;
		} axes[2];
		uint32_t axisSource;
	};

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
		xdg_wm_base* xdgWMBase { nullptr };
		wl_output* wlOutput { nullptr };
		wl_seat* wlSeat { nullptr };

		// Objects

		/* Surfaces */
		wl_surface* wlSurface { nullptr };
		xdg_surface* xdgSurface { nullptr };
		xdg_toplevel* xdgToplevel { nullptr };

		/* Input */
		wl_keyboard* wlKeyboard { nullptr };
		wl_pointer* wlPointer { nullptr };
		PointerEvent pointerEvent;
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

	// -- wl pointer

	InternalScope void wlPointerEnter
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t serial,
		wl_surface *surface,
		wl_fixed_t x,
		wl_fixed_t y
	)
	{
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(surface);

		Window* window = reinterpret_cast<Window*>(data);
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

		windowState->pointerEvent.eventMask |= POINTER_EVENT_ENTER;
		windowState->pointerEvent.serial = serial;
		windowState->pointerEvent.x = x;
		windowState->pointerEvent.y = y;
	}

	InternalScope void wlPointerLeave
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t serial,
		wl_surface *surface
	)
	{
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(surface);

		Window* window = reinterpret_cast<Window*>(data);
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

		windowState->pointerEvent.serial = serial;
		windowState->pointerEvent.eventMask |= POINTER_EVENT_LEAVE;
	}
	
	InternalScope void wlPointerMotion
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t time,
		wl_fixed_t x,
		wl_fixed_t y
	)
	{
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(time);

		Window* window = reinterpret_cast<Window*>(data);
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

		windowState->pointerEvent.eventMask |= POINTER_EVENT_MOTION;
		windowState->pointerEvent.x = x;
		windowState->pointerEvent.y = y;
	}

	InternalScope void wlPointerButton
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t serial,
		uint32_t time,
		uint32_t button,
		uint32_t state
	)
	{
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(time);

		Window* window = reinterpret_cast<Window*>(data);
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

		windowState->pointerEvent.serial = serial;
		windowState->pointerEvent.eventMask |= POINTER_EVENT_BUTTON;
		windowState->pointerEvent.button = button;
		windowState->pointerEvent.state = state;
	}

	InternalScope void wlPointerAxis
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t time,
		uint32_t axis,
		wl_fixed_t value
	)
	{
		CIN_UNUSED(data);
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(time);
		CIN_UNUSED(axis);
		CIN_UNUSED(value);
	}

	InternalScope void wlPointerAxisSource
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t axisSource
	)
	{
		CIN_UNUSED(data);
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(axisSource);
	}	

	InternalScope void wlPointerAxisStop
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t time,
		uint32_t axis
	)
	{
		CIN_UNUSED(data);
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(time);
		CIN_UNUSED(axis);
	}

	InternalScope void wlPointerAxisDiscrete
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t axis,
		int32_t discrete
	)
	{
		CIN_UNUSED(data);
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(axis);
		CIN_UNUSED(discrete);
	}

	InternalScope void wlPointerFrame
	(
		void *data,
		wl_pointer *wlPointer
	)
	{
		CIN_UNUSED(wlPointer);

		Window* window = reinterpret_cast<Window*>(data);
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());
		PointerEvent* pointerEvent = &windowState->pointerEvent;

		/* if(pointerEvent->eventMask & POINTER_EVENT_ENTER)
			CIN_TRACE("Pointer entered the surface at {0}, {1}", wl_fixed_to_double(pointerEvent->x), wl_fixed_to_double(pointerEvent->x)); */

		/* if(pointerEvent->eventMask & POINTER_EVENT_LEAVE)
			CIN_TRACE("Pointer left the surface"); */

		if(pointerEvent->eventMask & POINTER_EVENT_MOTION)
		{
			/*CIN_TRACE("Current cursor position: ({0}, {1})", wl_fixed_to_int(pointerEvent->x), wl_fixed_to_int(pointerEvent->y)); */
			AddMousePositionEventGUI((float)wl_fixed_to_double(pointerEvent->x), (float)wl_fixed_to_double(pointerEvent->y));
		}

		if(pointerEvent->eventMask & POINTER_EVENT_BUTTON)
		{
			const char* state = pointerEvent->state == WL_POINTER_BUTTON_STATE_PRESSED ? "Pressed" : "Released";
			CIN_UNUSED(state);
			//CIN_WARN("{} {}", state, (int)pointerEvent->button);
			//if(false)
			AddMouseButtonEventGUI(static_cast<MouseCode>(pointerEvent->button), pointerEvent->state == WL_POINTER_BUTTON_STATE_PRESSED);
		}

		/* Todo:
			add xdg_toplevel_resize & add xdg_toplevel_move
			must use !decorations! for this because on mouse button press we dont get pointer position
		*/

		memset(pointerEvent, 0, sizeof(PointerEvent));
	}

	InternalScope void wlPointerAxisValue120
	(
		void *data,
		wl_pointer *wlPointer,
		uint32_t axis,
		int32_t value120
	)
	{
		CIN_UNUSED(data);
		CIN_UNUSED(wlPointer);
		CIN_UNUSED(axis);
		CIN_UNUSED(value120);
	}

	InternalScope constexpr wl_pointer_listener wlPointerListener
	{
		.enter = wlPointerEnter,
		.leave = wlPointerLeave,
		.motion = wlPointerMotion,
		.button = wlPointerButton,
       	.axis = wlPointerAxis,
		.frame = wlPointerFrame,
       	.axis_source = wlPointerAxisSource,
       	.axis_stop = wlPointerAxisStop,
       	.axis_discrete = wlPointerAxisDiscrete,	
		.axis_value120 = wlPointerAxisValue120
	};

	// wl pointer --

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

		uint32_t* key;
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

			AddKeyEventGUI(key, true);
		}
		else
		{
			KeyReleasedEvent event(key);
			window->SendEvent(event);

			AddKeyEventGUI(key, false);
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

		bool pointerCapability { bool(capabilities & WL_SEAT_CAPABILITY_POINTER) };
		if(pointerCapability && windowState->wlPointer == nullptr)
		{
			windowState->wlPointer = wl_seat_get_pointer(windowState->wlSeat);
            wl_pointer_add_listener(windowState->wlPointer, &wlPointerListener, window);
		}
		else if(!pointerCapability && windowState->wlPointer != nullptr)
		{
			wl_pointer_release(windowState->wlPointer);
			windowState->wlPointer = nullptr;
		}

		bool keyboardCapability { bool(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) };
		if(keyboardCapability && windowState->wlKeyboard == nullptr)
		{
			windowState->wlKeyboard = wl_seat_get_keyboard(windowState->wlSeat);
			wl_keyboard_add_listener(windowState->wlKeyboard, &wlKeyboardListener, window);
		}
		else if(!keyboardCapability && windowState->wlKeyboard != nullptr)
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
	InternalScope void xdgWMBasePingHandler
	(
	    void *data,
	    xdg_wm_base* xdgWMBase,
	    uint32_t serial
	)
	{
	    (void)data;

		xdg_wm_base_pong(xdgWMBase, serial);
	}

	InternalScope constexpr xdg_wm_base_listener xdgWMBaseListener
	{
	    .ping = xdgWMBasePingHandler
	};
	// xdg shell --

	// -- xdg surface
	InternalScope void xdgSurfaceConfigureHandler
	(
	    void* data,
	    xdg_surface* xdgSurface,
	    uint32_t serial
	)
	{
	    xdg_surface_ack_configure(xdgSurface, serial);

	    Window* window = reinterpret_cast<Window*>(data);
		WindowProperties& windowProperties { window->GetProperties() };

		windowProperties.Mode = pending.mode;
		windowProperties.Focused = pending.focused;

		if(windowProperties.Width != pending.width || windowProperties.Height != pending.height)
		{
			windowProperties.Width = pending.width;
			windowProperties.Height = pending.height;

			WindowResizedEvent event(window, windowProperties.Width, windowProperties.Height);
			window->SendEvent(event);
		}
	}

	InternalScope xdg_surface_listener xdgSurfaceListener
	{
	    .configure {
		[]
		(
			void* data,
			xdg_surface* xdgSurface,
			uint32_t serial
		)
		{
			xdg_surface_ack_configure(xdgSurface, serial);

			Window* window = reinterpret_cast<Window*>(data);
			WindowProperties& windowProperties { window->GetProperties() };

			windowProperties.Mode = pending.mode;
			windowProperties.Focused = pending.focused;

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
	    void* data,
	    xdg_toplevel *xdgToplevel,
	    int32_t width,
	    int32_t height,
	    wl_array* states
	)
	{
	    (void)xdgToplevel;

	    Window* window = reinterpret_cast<Window*>(data);
		WindowProperties& windowProperties { window->GetProperties() };

		pending.mode = EWindowMode::Windowed;
		pending.focused = false;

	    xdg_toplevel_state* state;
	    wl_array_for_each_casted(state, states, xdg_toplevel_state*) // c++ being a crybaby about assigning void* to zxdg_toplevel_state*
	    {
	        switch(*state)
	        {
	        case XDG_TOPLEVEL_STATE_ACTIVATED:
				pending.focused = false;
	            break;

	        case XDG_TOPLEVEL_STATE_FULLSCREEN:
				pending.mode = EWindowMode::Fullscreen;
	            break;

	        case XDG_TOPLEVEL_STATE_MAXIMIZED:
				pending.mode = EWindowMode::Maximized;
	            break;

	        case XDG_TOPLEVEL_STATE_RESIZING:
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
	    void* data,
	    xdg_toplevel* xdgToplevel
	)
	{
	    (void)xdgToplevel;

	    CIN_TRACE("xdg-shell: Received close event");

	    Window* window = reinterpret_cast<Window*>(data);

	    WindowClosedEvent event(window);
	    window->SendEvent(event);
	}

	InternalScope void xdgToplevelConfigureBoundsHandler
	(
		void* data,
		struct xdg_toplevel* xdgToplevel,
		int32_t width,
		int32_t height
	)
	{
		CIN_UNUSED(data);
		CIN_UNUSED(xdgToplevel);
		CIN_UNUSED(width);
		CIN_UNUSED(height);

		CIN_INFO("xdg_toplevel_listener.configure_bounds: bounds: width = {0}, height = {1}", width, height);
	}

	InternalScope void xdgToplevelWMCapabilities
	(
		void* data,
		struct xdg_toplevel* xdgToplevel,
		struct wl_array* capabilities
	)
	{
		CIN_UNUSED(data);
		CIN_UNUSED(xdgToplevel);
		CIN_UNUSED(capabilities);

		CIN_INFO("Capabilities:");

		uint32_t* capability;
		wl_array_for_each_casted(capability, capabilities, uint32_t*)
		{
			CIN_INFO("{0}", *capability);
		}
	}

	InternalScope constexpr xdg_toplevel_listener xdgToplevelListener
	{
	    .configure = xdgToplevelConfigureHandler,
	    .close = xdgToplevelCloseHandler,
		.configure_bounds = xdgToplevelConfigureBoundsHandler,
		.wm_capabilities =xdgToplevelWMCapabilities
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
	    if(strcmp(interface, wl_compositor_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->wlCompositor =
	        (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version);

	        if(wl_compositor_interface.version != int(version))
	            CIN_INFO("Using wl_compositor_interface version {0} but the desired version is {1}", version, wl_compositor_interface.version);
	    }
	    else if(strcmp(interface, xdg_wm_base_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->xdgWMBase =
	        (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, version);

        	/* xdg_wm_base_add_listener(reinterpret_cast<PlatformWindowState*>(data)->xdgWMBase, &xdgWMBaseListener, NULL); */

	        if(xdg_wm_base_interface.version != int(version))
	            CIN_INFO("Using xdg_wm_base_interface version {0} but the desired version is {1}", version, xdg_wm_base_interface.version);
	    }
	    else if(strcmp(interface, wl_output_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->wlOutput =
	        (wl_output*)wl_registry_bind(registry, name, &wl_output_interface, version);

	        if(wl_output_interface.version != int(version))
	            CIN_INFO("Using wl_output_interface version {0} but the desired version is {1}", version, wl_output_interface.version);
	    }
	    else if(strcmp(interface, wl_seat_interface.name) == 0)
	    {
	        reinterpret_cast<PlatformWindowState*>(data)->wlSeat =
	        (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, version);

	        if(wl_seat_interface.version != int(version))
	            CIN_INFO("Using wl_seat_interface version {0} but the desired version is {1}", version, wl_seat_interface.version);
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
		CIN_WARN("add decorations, handle rest of xdg_toplevel callbacks!");
		CIN_ERROR("add decorations, handle rest of xdg_toplevel callbacks!");
		CIN_WARN("add decorations, handle rest of xdg_toplevel callbacks!");
		CIN_ERROR("add decorations, handle rest of xdg_toplevel callbacks!");
		CIN_WARN("add decorations, handle rest of xdg_toplevel callbacks!");

		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

	    if(!windowState->xdgWMBase)
		{
			CIN_CRITICAL("Couldn't find xdg_wm_base resource, make sure your compositor supports xdg-shell extension");
			CIN_PANIC_EXIT();
		}

        windowState->wlSurface = wl_compositor_create_surface(windowState->wlCompositor);
        wl_surface_add_listener(windowState->wlSurface, &wlSurfaceListener, window);

		wl_callback* fCallback = wl_surface_frame(windowState->wlSurface);
		wl_callback_add_listener(fCallback, &wlSurfaceFrameListener, window);

        windowState->xdgSurface = xdg_wm_base_get_xdg_surface(windowState->xdgWMBase, windowState->wlSurface);
        windowState->xdgToplevel = xdg_surface_get_toplevel(windowState->xdgSurface);

        xdg_wm_base_add_listener(windowState->xdgWMBase, &xdgWMBaseListener, NULL);
        xdg_surface_add_listener(windowState->xdgSurface, &xdgSurfaceListener, window);
        xdg_toplevel_add_listener(windowState->xdgToplevel, &xdgToplevelListener, window);

        wl_surface_commit(windowState->wlSurface);
	}

	InternalScope CIN_FORCE_INLINE void WaylandSetup(Window* window)
	{
		PlatformWindowState* windowState = const_cast<PlatformWindowState*>(window->GetState());

	    windowState->wlDisplay = wl_display_connect(NULL);
	    if(!windowState->wlDisplay)
		{
			CIN_CRITICAL("Couldn't connect to the wayland display");
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
		m_State(cinew PlatformWindowState),
		m_InputState(cinew InputState),
		m_Properties(std::move(windowProperties)),
        m_EventCallback(callback)
	{
		if(m_Properties.Mode != EWindowMode::Unspecified)
			CIN_INFO("The compositor is going to decide on the window's mode");

        WaylandSetup(this);

		SetName(m_Properties.Name);
	}

	Window::~Window() noexcept
	{
		cindel m_InputState;
		cindel m_State;
	}

	void Window::PollEvents()
	{
		wl_display_dispatch_pending(m_State->wlDisplay);

		bool flushFailure { false };
		while(wl_display_flush(m_State->wlDisplay) == -1)
		{
			CIN_ERROR("Failed flushing the display");

			if(errno != EAGAIN)
			{
				flushFailure = true;
				break;
			}

			pollfd fd { };
			fd.fd = wl_display_get_fd(m_State->wlDisplay);
			fd.events = POLLOUT;

			while(poll(&fd, 1, -1) == -1)
			{
				if(errno != EAGAIN)
				{
					flushFailure = true;
					break;
				}
			}
		}

		if(flushFailure)
		{
			CIN_CRITICAL("Could not recover");
			CIN_PANIC_EXIT();
		}
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

	const WindowProperties& Window::GetProperties() const
	{
		return m_Properties;
	}

	const InputState* Window::GetInputState() const
	{
		return m_InputState;
	}

	const void* Window::GetNativeHandle() const
	{
		CIN_UNIMPLEMENTED(); return nullptr;
	}

	void Window::SetName(const char* windowName)
	{
		m_Properties.Name = windowName;
		xdg_toplevel_set_title(m_State->xdgToplevel, m_Properties.Name);
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
					xdg_toplevel_set_fullscreen(m_State->xdgToplevel, m_State->wlOutput);
					m_Properties.Mode = EWindowMode::Fullscreen;
				}

				break;
			}

			case EWindowMode::Maximized:
			{
				if(m_Properties.Mode != EWindowMode::Maximized)
				{
					xdg_toplevel_set_maximized(m_State->xdgToplevel);
					m_Properties.Mode = EWindowMode::Maximized;
				}

				break;
			}

			case EWindowMode::Windowed:
			{
				if(m_Properties.Mode != EWindowMode::Windowed)
				{
					if(m_Properties.Mode == EWindowMode::Fullscreen)
						xdg_toplevel_unset_fullscreen(m_State->xdgToplevel);

					else if(m_Properties.Mode == EWindowMode::Maximized)
						xdg_toplevel_unset_maximized(m_State->xdgToplevel);

					/* m_Properties.Mode = EWindowMode::Windowed; */
				}

				break;
			}

			default:
				break;
		}
	}

	void Window::SetSize(const std::pair<uint32_t, uint32_t> windowSize)
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