#include "Cinnamon/include/Core/Application.h"
#include "Cinnamon/include/Core/Logger.h"

#include <string>
#include <vector>

namespace Cinnamon {
	Application* Application::s_ApplicationInstance{ nullptr };

	Application::Application() noexcept
		:
		m_Window(nullptr)
	{
		CIN_ASSERT(s_ApplicationInstance == nullptr, "Application already initialized!");
		s_ApplicationInstance = this;
	}

	Application::~Application() noexcept
	{
		delete m_Window;
	}

	bool Application::Initialize()
	{
		m_Window = new Window(WindowProperties{ u8"Cinnamon Application", 800U, 600U, EWindowMode::Unspecified });

<<<<<<< HEAD
		CIN_TRACE("Logger test, {0}, {1}, {2}", 1, 2, "Trace");
		CIN_INFO("Logger test, {0}, {1}, {2}", 1, 2, "Info");
		CIN_WARN("Logger test, {0}, {1}, {2}", 1, 2, "Warn");
		CIN_ERROR("Logger test, {0}, {1}, {2}", 1, 2, "Error");
		CIN_CRITICAL("Logger test, {0}, {1}, {2}", 1, 2, "Critical");

=======
>>>>>>> 648020f940992a6d41b556da433d740f8120fd03
		return true;
	}

	bool Application::Run()
	{
		return true;
	}

	bool Application::Shutdown()
	{
		return true;
	}

	const Application* Application::Get()
	{
		/* TODO: Check if valid */
		return s_ApplicationInstance;
	}
}