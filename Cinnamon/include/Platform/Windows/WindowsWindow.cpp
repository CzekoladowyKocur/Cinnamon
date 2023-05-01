#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Core/Input.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/KeyEvent.h"
#include "Cinnamon/include/Event/MouseEvent.h"

/* Assumes variable "event" of type Event& is in scope */
#define LOG_UNHANDLED_EVENT(eventType)									\
{																		\
	const eventType* cast{ dynamic_cast<const eventType*>(&event) };	\
	if (cast)															\
	{																	\
	CIN_WARN("Unhandled event: {0}", #eventType);						\
	}																	\
}

/* NOTE: Temp */
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Cinnamon {
	InternalScope LRESULT CALLBACK Windows32ProcessMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam);
	InternalScope void DefaultEventCallback(const Event& event);

	//constexpr CHAR WIN32_API_WINDOW_CLASS_NAME[] = "CINNAMON_ENGINE_WINDOW_CLASS";

	/* Declared in Window.h */
	struct PlatformWindowState
	{
		HWND Handle{ nullptr };
		HINSTANCE Instance{ nullptr };
		STL::String WindowClassName;

		uint32_t StyleFlags{ 0U };
		uint32_t ExtendedStyleFlags{ 0U };
	};

	Window::Window(WindowProperties&& windowProperties, const EventCallbackFunction callback) noexcept
		:
		m_State(nullptr),
		m_InputState(cinew InputState),
		m_Properties(std::move(windowProperties)),
		m_EventCallback(callback ? callback : DefaultEventCallback)
	{
		const HINSTANCE hInstance{ GetModuleHandle(NULL) };
		CIN_VERIFY(hInstance);

		const std::string windowClassName{ "CinnamonWC-" + Platform::GenerateUUID() };
		CIN_TRACE("Generated window class name: {}", windowClassName);
		/* Initialize win32 window class, only generic one for now */
		const WNDCLASSEX windowClass
		{
			.cbSize{ sizeof(WNDCLASSEX)},
			.style
			{
				CS_DBLCLKS | /* Sends message for double clicks */
				CS_HREDRAW | /* Redraw window if width has changed */
				CS_VREDRAW | /* Redraw window if height has changed */
				CS_OWNDC /* Application owns a device context */
			},
			.lpfnWndProc{ Windows32ProcessMessage },
			.cbClsExtra{ 0U },
			.cbWndExtra{ 0U },
			.hInstance{ hInstance },
			.hIcon{ LoadIcon(NULL, IDI_APPLICATION) },
			.hCursor{ LoadCursor(NULL, IDC_ARROW) },
			.hbrBackground{ reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)) },
			.lpszMenuName{ nullptr },
			.lpszClassName{ windowClassName.c_str() },
			.hIconSm{ windowClass.hIcon },
		};

		/* Register window class */
		if (!RegisterClassExA(&windowClass))
		{
			MessageBox(NULL, "Engine window class registration failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
			CIN_PANIC_EXIT();
		}

		/* Initialize window */
		m_State = cinew PlatformWindowState
		{
			.Handle{ nullptr }, /* Set later */
			.Instance{ hInstance },
			.StyleFlags
			{ 
				WS_OVERLAPPEDWINDOW | /* The window is an overlapped window. An overlapped window has a title bar and a border. Same as the WS_TILED style. */
				WS_SYSMENU			| /* The window has a window menu on its title bar. The WS_CAPTION style must also be specified. */
				WS_CAPTION			| /* The window has a title bar (includes the WS_BORDER style). */
				WS_MAXIMIZEBOX		|
				WS_MINIMIZEBOX		|
				WS_THICKFRAME
			},
			.ExtendedStyleFlags{ WS_EX_APPWINDOW }, /* Forces a top-level window onto the taskbar when the window is visible. */
		};

		/* Obtain the size of border */
		RECT windowBorderRectangle{ 0U, 0U, 0U, 0U };
		if (!AdjustWindowRectEx(&windowBorderRectangle, m_State->StyleFlags, 0, m_State->ExtendedStyleFlags))
		{
			MessageBox(NULL, "Failed to obtain size of window border", "Error!", MB_ICONEXCLAMATION | MB_OK);
			CIN_PANIC_EXIT();
		}

		const HWND windowHandle{ CreateWindowEx(
			m_State->ExtendedStyleFlags, /* extended styles */
			reinterpret_cast<LPCSTR>(windowClassName.c_str()),
			reinterpret_cast<LPCSTR>(m_Properties.Name),
			m_State->StyleFlags, /* basic styles */
			0,
			0,
			m_Properties.Width,
			m_Properties.Height,
			NULL, /* parent window */
			NULL, /* lpmenu */
			reinterpret_cast<HINSTANCE>(hInstance), /* hinstance */
			this /* User data */
		) };

		if (!windowHandle)
		{
			MessageBox(NULL, "Failed to create window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
			CIN_PANIC_EXIT();
		}

		m_State->Handle = windowHandle;
		m_State->WindowClassName = windowClassName;
		/* TODO: if the window should not accept input, this should be false */
		const bool shouldActivate{ TRUE };
		const int32_t showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;

		ShowWindow(windowHandle, showWindowCommandFlags);
		if (!SetForegroundWindow(windowHandle))
		{
			MessageBox(NULL, "Failed to set foreground window", "Error!", MB_ICONEXCLAMATION | MB_OK);
			CIN_PANIC_EXIT();
		}

		if (!SetFocus(windowHandle))
		{
			MessageBox(NULL, "Failed to focus window", "Error!", MB_ICONEXCLAMATION | MB_OK);
			CIN_PANIC_EXIT();
		}

		if (m_Properties.Mode != EWindowMode::Unspecified)
		{
			const EWindowMode windowMode{ m_Properties.Mode };
			m_Properties.Mode = EWindowMode::Unspecified;
			SetWindowMode(windowMode);
		}
	}

	Window::~Window() noexcept
	{
		CIN_ASSERT(m_State, "Invalid internal window state");
		/* TODO: Move to event proccessing? ... */
		if (!DestroyWindow(m_State->Handle))
		{
			MessageBox(NULL, "Failed to destroy window", "Error!", MB_ICONEXCLAMATION | MB_OK);
			CIN_PANIC_EXIT();
		}

		CIN_VERIFY(UnregisterClassA(m_State->WindowClassName.c_str(), GetModuleHandle(NULL)));
		cindel m_InputState;
		cindel m_State;
	}

	void Window::PollEvents()
	{
		MSG message;
		PeekMessage(&message, NULL, NULL, NULL, PM_REMOVE);
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		InvalidateRect(m_State->Handle, NULL, TRUE);
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

	EventCallbackFunction Window::GetEventCallback() const
	{
		return m_EventCallback;
	}

	const PlatformWindowState* Window::GetState() const
	{
		return m_State;
	}

	const InputState* Window::GetInputState() const
	{
		return m_InputState;
	}

	WindowProperties& Window::GetProperties()
	{
		return m_Properties;
	}

	const WindowProperties& Window::GetProperties() const
	{
		return m_Properties;
	}

	const void* Window::GetNativeHandle() const
	{
		return m_State->Handle;
	}

	void Window::SetName(const char* windowName)
	{
		m_Properties.Name = windowName;
	}

	void Window::SetWidth(const uint32_t windowWidth)
	{
		CIN_UNIMPLEMENTED();
		m_Properties.Width = windowWidth;
	}

	void Window::SetHeight(const uint32_t windowHeight)
	{
		CIN_UNIMPLEMENTED();
		m_Properties.Height = windowHeight;
	}

	void Window::SetWindowMode(const EWindowMode windowMode)
	{
		[[unlikely]]
		if (m_Properties.Mode == windowMode)
			return;

		const EWindowMode currentWindowMode{ m_Properties.Mode };
		const HWND windowHandle{ m_State->Handle };
		const LONG windowStyle{ static_cast<LONG>(m_State->StyleFlags) };
		const LONG windowStyleExtended{ static_cast<LONG>(m_State->ExtendedStyleFlags) };
		
		switch (windowMode)
		{
			case EWindowMode::Unspecified:
			case EWindowMode::Windowed:
			{
				if (currentWindowMode == EWindowMode::Fullscreen)
				{
					CIN_VERIFY(SetWindowLong(
						windowHandle,
						GWL_STYLE,
						windowStyle));

					CIN_VERIFY(SetWindowLong(
						windowHandle,
						GWL_EXSTYLE,
						windowStyleExtended));
				}

				ShowWindow(
					windowHandle, 
					SW_NORMAL);

				break;
			}

			case EWindowMode::Maximized:
			{
				if (currentWindowMode == EWindowMode::Fullscreen)
				{
					CIN_VERIFY(SetWindowLong(
						windowHandle,
						GWL_STYLE,
						windowStyle));

					CIN_VERIFY(SetWindowLong(
						windowHandle,
						GWL_EXSTYLE,
						windowStyleExtended));
				}

				ShowWindow(
					windowHandle,
					SW_SHOWMAXIMIZED);

				break;
			}

			case EWindowMode::Fullscreen:
			{
				CIN_VERIFY(SetWindowLong(
					windowHandle,
					GWL_STYLE,
					windowStyle & ~(WS_CAPTION | WS_THICKFRAME)));

				CIN_VERIFY(SetWindowLong(
					windowHandle,
					GWL_EXSTYLE,
					windowStyleExtended & ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE)));

				MONITORINFO monitorInformation
				{
					.cbSize { sizeof(MONITORINFO) },
					.rcMonitor { 0U, 0U, 0U, 0U },
					.rcWork { 0U, 0U, 0U, 0U },
					.dwFlags { 0U }
				};

				CIN_VERIFY(GetMonitorInfo(
					MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST), 
					&monitorInformation));

				RECT clientRectangle
				{
					.left { 0U },
					.top { 0U },
					.right { 0U },
					.bottom { 0U },
				};

				CIN_VERIFY(GetClientRect(
					windowHandle, 
					&clientRectangle));

				CIN_VERIFY(SetWindowPos(
					windowHandle,
					NULL,
					clientRectangle.left,
					clientRectangle.top,
					m_Properties.Width,
					m_Properties.Height,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED));

				SendMessage(
					windowHandle, 
					WM_SYSCOMMAND, 
					SC_MAXIMIZE, 
					0);

				break;
			}
		}

		m_Properties.Mode = windowMode;
	}

	void Window::SetSize(const std::pair<uint32_t, uint32_t> windowSize)
	{
		CIN_UNIMPLEMENTED();
		m_Properties.Width = windowSize.first;
		m_Properties.Height = windowSize.second;
	}

	void Window::SetEventCallback(const EventCallbackFunction callback)
	{
		m_EventCallback = callback;
	}

	LRESULT CALLBACK Windows32ProcessMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam)
	{
		/* NOTE: Temp */
		ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam);
		Window* window{ reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) };

		switch (message)
		{
			case WM_CREATE:
			{
				CREATESTRUCT* createStruct{ reinterpret_cast<CREATESTRUCT*>(lParam) };
				window = reinterpret_cast<Window*>(createStruct->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

				POINT point;
				CIN_VERIFY(GetCursorPos(&point));
				window->m_InputState->SetMouseCursorPosition(static_cast<uint32_t>(point.x), static_cast<uint32_t>(point.y));

				return DefWindowProcA(hwnd, message, wParam, lParam);
			}
		
			case WM_ACTIVATE:
			{
				return 0;
			}

			case WM_PAINT:
			{
				ApplicationRenderEvent event;
				window->SendEvent(event);
				/* Notify the os we handled it */
				return 0;
			}

			case WM_ERASEBKGND:
			{
				/* Notify the OS that erasing will be handled by the application to prevent flicker. */
				return 1;
			}

			case WM_SIZE:
			{
				{
					const uint32_t width{ LOWORD(lParam) };
					const uint32_t height{ HIWORD(lParam) };

					window->m_Properties.Width = width;
					window->m_Properties.Height = height;

					WindowResizedEvent event(window, width, height);
					window->SendEvent(event);
				}
				/* Redraw the window to make resizing smooth */
				{
					ApplicationRenderEvent event;
					window->SendEvent(event);
				}

				return 0;
			}

			case SC_MAXIMIZE:
			{
				return 0;
			}

			case WM_CLOSE:
			{
				WindowClosedEvent event(window);
				window->SendEvent(event);

				return 0;
			}

			case WM_QUIT:
			{
				return 0;
			}
			
			case WM_DESTROY:
			{
				return 0;
			}

			case WM_KEYUP: /* Nonsystem key */
			case WM_SYSKEYUP: /* System key (alt pressed) */
			{
				/* TODO: Add repeated key presses */
				window->m_InputState->SetKeyState(static_cast<Key>(wParam), EKeyState::Released);

				return 0;
			}

			case WM_KEYDOWN: /* Nonsystem key */
			case WM_SYSKEYDOWN: /* System key (alt pressed) */
			{
				window->m_InputState->SetKeyState(static_cast<Key>(wParam), EKeyState::Pressed);

				KeyPressedEvent event(static_cast<KeyCode>(wParam));
				window->SendEvent(event);

				return 0;
			}

			case WM_LBUTTONDOWN:
			{
				window->m_InputState->SetMouseButtonState(Mouse::LeftButton, EMouseState::Pressed);
				return 0;
			}

			case WM_MBUTTONDOWN:
			{
				window->m_InputState->SetMouseButtonState(Mouse::MiddleButton, EMouseState::Pressed);
				return 0;
			}

			case WM_RBUTTONDOWN:
			{
				window->m_InputState->SetMouseButtonState(Mouse::RightButton, EMouseState::Pressed);
				return 0;
			}


			case WM_LBUTTONUP:
			{
				window->m_InputState->SetMouseButtonState(Mouse::LeftButton, EMouseState::Released);
				return 0;
			}

			case WM_MBUTTONUP:
			{
				window->m_InputState->SetMouseButtonState(Mouse::MiddleButton, EMouseState::Released);
				return 0;
			}

			case WM_RBUTTONUP:
			{
				window->m_InputState->SetMouseButtonState(Mouse::RightButton, EMouseState::Released);
				return 0;
			}

			case WM_MOUSEMOVE:
			{
				const uint32_t xPosition = static_cast<uint32_t>(GET_X_LPARAM(lParam));
				const uint32_t yPosition = static_cast<uint32_t>(GET_Y_LPARAM(lParam));

				window->m_InputState->SetMouseCursorPosition(xPosition, yPosition);
				return 0;
			}
		}

		return DefWindowProcA(hwnd, message, wParam, lParam);
	}

	void DefaultEventCallback(const Event& event)
	{
		/* Window Events */
		LOG_UNHANDLED_EVENT(WindowClosedEvent);
		LOG_UNHANDLED_EVENT(WindowMinimizedEvent);
		LOG_UNHANDLED_EVENT(WindowMaximizedEvent);
		LOG_UNHANDLED_EVENT(WindowResizedEvent);
		LOG_UNHANDLED_EVENT(WindowSurfaceUpdatedEvent);
		/* Application Events */
		LOG_UNHANDLED_EVENT(ApplicationTickEvent);
		LOG_UNHANDLED_EVENT(ApplicationRenderEvent);
		/* Key events */
		LOG_UNHANDLED_EVENT(KeyPressedEvent);
		LOG_UNHANDLED_EVENT(KeyReleasedEvent);
		LOG_UNHANDLED_EVENT(KeyHeldEvent);
		/* Mouse events */
		LOG_UNHANDLED_EVENT(MousePressedEvent);
		LOG_UNHANDLED_EVENT(MouseReleasedEvent);
		LOG_UNHANDLED_EVENT(MouseScrolledEvent);
	}
}
#endif