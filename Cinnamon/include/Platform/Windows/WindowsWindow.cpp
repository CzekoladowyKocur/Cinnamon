#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"

namespace Cinnamon {
	InternalScope LRESULT CALLBACK Windows32ProcessMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam);
	InternalScope std::once_flag s_Win32ClassInitialized;

	constexpr CHAR WIN32_API_WINDOW_CLASS_NAME[] = "CINNAMON_ENGINE_WINDOW_CLASS";

	/* Declared in Window.h */
	struct PlatformWindowState
	{
		HWND Handle{ nullptr };
		HINSTANCE Instance{ nullptr };

		uint32_t StyleFlags{ 0U };
		uint32_t ExtendedStyleFlags{ 0U };
	};

	Window::Window(WindowProperties&& windowProperties, const EventCallbackFunction callback) noexcept
		:
		m_State(nullptr),
		m_Properties(std::move(windowProperties)),
		m_EventCallback(callback)
	{
		const HINSTANCE hInstance{ GetModuleHandle(NULL) };
		/* Initialize win32 window class, only generic one for now */
		std::call_once(s_Win32ClassInitialized, [=]() {
			WNDCLASSEX windowClass;
			windowClass.lpszClassName = WIN32_API_WINDOW_CLASS_NAME;
			windowClass.lpszMenuName = NULL;
			windowClass.hInstance = reinterpret_cast<HINSTANCE>(hInstance);
			windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			windowClass.hIconSm = windowClass.hIcon;
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowClass.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
			windowClass.style =
				CS_DBLCLKS | /* Sends message for double clicks */
				CS_HREDRAW | /* Redraw window if width has changed */
				CS_VREDRAW,/* Redraw window if height has changed */
			//	CS_OWNDC;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.lpfnWndProc = Windows32ProcessMessage;

			/* Register window class */
			if (!RegisterClassExA(&windowClass))
			{
				MessageBox(NULL, "Engine window class registration failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
				CIN_PANIC_EXIT();
			}
			});

		/* Initialize window */
		m_State = CIN_NEW();
		m_State->Instance = hInstance;
		m_State->StyleFlags =
			WS_OVERLAPPEDWINDOW | /* The window is an overlapped window. An overlapped window has a title bar and a border. Same as the WS_TILED style. */
			WS_SYSMENU | /* The window has a window menu on its title bar. The WS_CAPTION style must also be specified. */
			WS_CAPTION | /* The window has a title bar (includes the WS_BORDER style). */
			WS_MAXIMIZEBOX |
			WS_MINIMIZEBOX |
			WS_THICKFRAME;

		m_State->ExtendedStyleFlags =
			WS_EX_APPWINDOW /* Forces a top-level window onto the taskbar when the window is visible. */;

		/* Obtain the size of border */
		RECT windowBorderRectangle{ 0, 0, 0, 0 };
		if (!AdjustWindowRectEx(&windowBorderRectangle, m_State->StyleFlags, 0, m_State->ExtendedStyleFlags))
		{
			MessageBox(NULL, "Failed to obtain size of window border", "Error!", MB_ICONEXCLAMATION | MB_OK);
			CIN_PANIC_EXIT();
		}

		const HWND windowHandle{ CreateWindowEx(
			m_State->ExtendedStyleFlags, /* extended styles */
			reinterpret_cast<LPCSTR>(WIN32_API_WINDOW_CLASS_NAME),
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
		/* TODO: if the window should not accept input, this should be false */
		const bool shouldActivate{ TRUE };
		int32_t showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;

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

		CIN_DELETE(m_State);
	}

	void Window::PollEvents()
	{
		MSG message;
		PeekMessage(&message, m_State->Handle, NULL, NULL, PM_REMOVE);
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
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

	const void* Window::GetNativeHandle() const
	{
		return m_State->Handle;
	}

	void Window::SetName(const char8_t* windowName)
	{
		m_Properties.Name = windowName;
	}

	void Window::SetWidth(const uint32_t windowWidth)
	{
		m_Properties.Width = windowWidth;
	}

	void Window::SetHeight(const uint32_t windowHeight)
	{
		m_Properties.Height = windowHeight;
	}

	void Window::SetWindowMode(const EWindowMode windowMode)
	{
		m_Properties.Mode = windowMode;
	}

	void Window::SetSize(std::pair<uint32_t, uint32_t> windowSize)
	{
		m_Properties.Width = windowSize.first;
		m_Properties.Height = windowSize.second;
	}

	LRESULT CALLBACK Windows32ProcessMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam)
	{
		Window* window{ reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) };

		switch (message)
		{
			case WM_CREATE:
			{
				CREATESTRUCT* createStruct{ reinterpret_cast<CREATESTRUCT*>(lParam) };
				window = reinterpret_cast<Window*>(createStruct->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

				return DefWindowProcA(hwnd, message, wParam, lParam);
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

			case WM_CLOSE:
			{
				WindowClosedEvent event(window);
				window->SendEvent(event);

				return 0;
			}

			case WM_DESTROY:
			{
				return DefWindowProcA(hwnd, message, wParam, lParam);
			}
		}

		return DefWindowProcA(hwnd, message, wParam, lParam);
	}
}
#endif