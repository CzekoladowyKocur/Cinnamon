#include "Cinnamon/include/Core/Application.h"

namespace Cinnamon {
	Application* Application::s_ApplicationInstance{ nullptr };

	Application::Application() noexcept
		:
		m_Window(nullptr)
	{
		/* TODO: Check if already initialized */
		s_ApplicationInstance = this;
	}

	Application::~Application() noexcept
	{
		delete m_Window;
	}

	bool Application::Initialize()
	{
		m_Window = new Window(WindowProperties{ u8"Cinnamon Application", 800U, 600U, EWindowMode::Unspecified });

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