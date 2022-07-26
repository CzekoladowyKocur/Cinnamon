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
		s_ApplicationInstance = this;
	}

	Application::~Application() noexcept
	{
		delete m_Window;
		DUMP_CINNAMON_ALLOCATIONS();
	}

	bool Application::Initialize()
	{
		m_Window = new Window(
			WindowProperties{ u8"Cinnamon Application", 800U, 600U, EWindowMode::Unspecified }, 
			std::bind(&Application::OnEvent, this, std::placeholders::_1));

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

		return true;
	}

	bool Application::Run()
	{
		while (m_Running)
		{
			m_Window->PollEvents();

			GraphicsContext::AcquireNextImage(m_Window);
			GraphicsContext::PresentImage(m_Window);
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
		dispatcher.Dispatch<WindowClosedEvent>(std::bind(&Application::OnWindowClosed, this, std::placeholders::_1));
	}

	bool Application::OnWindowClosed(WindowClosedEvent& event)
	{
		CIN_UNUSED(event);
		m_Running = false;

		return true;
	}

	const Application* Application::Get()
	{
		CIN_ASSERT(s_ApplicationInstance, "Application not instantiated");
		return s_ApplicationInstance;
	}
}