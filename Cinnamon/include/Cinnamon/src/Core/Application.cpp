#include "Cinnamon/include/Core/Application.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"

namespace Cinnamon {
	Application* Application::s_ApplicationInstance{ nullptr };

	Application::Application() noexcept
		:
		m_Running(true),
		m_Minimized(false),
		m_Window(nullptr)
	{
		CIN_ASSERT(s_ApplicationInstance == nullptr, "Application already initialized!");
		CIN_TRACE("Running cinnamon build {}", Platform::GetBuildDate());

		s_ApplicationInstance = this;
	}

	Application::~Application() noexcept
	{
		cindel m_Window;
		CIN_DUMP_ALLOCATIONS();
	}

	bool Application::Initialize()
	{
		/* TODO: Set window event callbacks after context creation? */
		m_Window = cinew Window(WindowProperties{ "Cinnamon Application", 800U, 600U, EWindowMode::Fullscreen });

		if (!GraphicsContext::Initialize())
		{
			CIN_CRITICAL("Failed to initialize graphics context");
			return false;
		}

		if (!GraphicsContext::CreateSurface(m_Window))
		{
			CIN_CRITICAL("Failed to set main window as graphics context");
			return false;
		}

		CIN_TRACE("Logger test, {0}, {1}, {2}", 1, 2, "Trace");
		CIN_INFO("Logger test, {0}, {1}, {2}", 1, 2, "Info");
		CIN_WARN("Logger test, {0}, {1}, {2}", 1, 2, "Warn");
		CIN_ERROR("Logger test, {0}, {1}, {2}", 1, 2, "Error");
		CIN_CRITICAL("Logger test, {0}, {1}, {2}", 1, 2, "Critical");

		m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
		return true;
	}

	bool Application::Run()
	{
		[[likely]]
		while (m_Running)
		{
			m_Window->PollEvents();
		}

		return true;
	}

	bool Application::Shutdown()
	{
		bool shutdownSuccessful{ true };

		if (!GraphicsContext::Shutdown())
		{
			CIN_CRITICAL("Failed to shutdown graphics context");
			shutdownSuccessful = false;
		}

		return shutdownSuccessful;
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<ApplicationRenderEvent>(std::bind(&Application::OnApplicationRender, this, std::placeholders::_1));
		dispatcher.Dispatch<WindowResizedEvent>(std::bind(&Application::OnWindowResized, this, std::placeholders::_1));
		dispatcher.Dispatch<WindowClosedEvent>(std::bind(&Application::OnWindowClosed, this, std::placeholders::_1));
		dispatcher.Dispatch<KeyPressedEvent>(std::bind(&Application::OnKeyPressed, this, std::placeholders::_1));
	}

	bool Application::OnApplicationRender(ApplicationRenderEvent& event)
	{
		CIN_UNUSED(event);

		/* Clear swapchain image for now */
		[[likely]]
		if (not m_Minimized)
		{
			GraphicsContext::AcquireNextImage(m_Window);
			GraphicsContext::PresentImage(m_Window);
		}

		return true;
	}

	bool Application::OnWindowResized(WindowResizedEvent& event)
	{
		const auto [width, height] { event.GetResize() };
		m_Minimized = (width == 0U) or (height == 0U);

		[[likely]]
		if(not m_Minimized)
			GraphicsContext::ResizeSurface(m_Window, width, height);
		
		return true;
	}

	bool Application::OnWindowClosed(WindowClosedEvent& event)
	{
		CIN_UNUSED(event);
		m_Running = false;

		return true;
	}
	bool Application::OnKeyPressed(KeyPressedEvent& event)
	{
		const Key key{ event.GetKey() };
		switch(key)
		{
			case Key::F10:
			{
				const EWindowMode currentWindowMode{ m_Window->GetWindowMode() };
				m_Window->SetWindowMode(currentWindowMode != EWindowMode::Maximized ? EWindowMode::Maximized : EWindowMode::Windowed);

				break;
			}

			case Key::F11:
			{
				const EWindowMode currentWindowMode{ m_Window->GetWindowMode() };
				m_Window->SetWindowMode(currentWindowMode != EWindowMode::Fullscreen ? EWindowMode::Fullscreen : EWindowMode::Maximized);

				break;
			}

			case Key::A:
			{
				CIN_CRITICAL("A IS NOT REAL");
				m_Window->SetName("A IS NOT REAL");
			}

			default:
				break;
		}

		/* Handled for now */
		return true;
	}

	const Application* Application::Get()
	{
		CIN_ASSERT(s_ApplicationInstance, "Application not instantiated");
		return s_ApplicationInstance;
	}
}