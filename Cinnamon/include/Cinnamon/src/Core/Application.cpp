#include "Cinnamon/include/Core/Application.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Core/LayerStack.h"
#include "Cinnamon/include/Event/Event.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/KeyEvent.h"
#include "Cinnamon/include/Event/MouseEvent.h"

namespace Cinnamon {
	Application* Application::s_ApplicationInstance{ nullptr };

	Application::Application() noexcept
		:
		m_Running(true),
		m_Minimized(false),
		m_Window(nullptr)
	{
		CIN_ASSERT(s_ApplicationInstance == nullptr, "Application already instantiated!");
		CIN_TRACE("Running cinnamon build {}", Platform::GetBuildDate());

		s_ApplicationInstance = this;
	}

	Application::~Application() noexcept
	{
		cindel m_LayerStack;
		cindel m_Window;
		CIN_DUMP_ALLOCATIONS();
	}

	bool Application::Initialize()
	{
		m_Window = cinew Window(WindowProperties{ "Cinnamon Application", 800U, 600U, EWindowMode::Windowed, true });
		m_LayerStack = cinew LayerStack;

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

		CIN_WARN("Queues from same families might be faster");
		m_Window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));

		if (!OnUserInitialize())
		{
			CIN_CRITICAL("Failed to initialize user data");
			return false;
		}

		return true;
	}

	bool Application::Run()
	{
		double lastFrameTime{ Platform::GetAbsoluteTime() };
		double timer{ 0.0 };
		uint32_t fpsCounter{ 0U };

		[[likely]]
		while (m_Running)
		{
			m_Window->PollEvents();
			const double currentTime{ Platform::GetAbsoluteTime() };
			const Timestep timestep{ static_cast<Timestep::Type>(currentTime - lastFrameTime) };
			
			/* Update layers */
			for (Layer* layer : *m_LayerStack)
				layer->OnUpdate(timestep);
			
			lastFrameTime = currentTime;
			/* Temporary */
			timer += timestep;
			[[unlikely]]
			if (timer > 1.0)
			{
				printf("FPS: %d\n", fpsCounter);
				fpsCounter = 0;
				timer = 0.0;
			}
			
			++fpsCounter;
		}

		return true;
	}

	bool Application::Shutdown()
	{
		bool shutdownSuccessful{ true };

		if (!OnUserShutdown())
		{
			CIN_CRITICAL("Failed to shutdown user data");
			shutdownSuccessful = false;
		}

		if (!GraphicsContext::Shutdown())
		{
			CIN_CRITICAL("Failed to shutdown graphics context");
			shutdownSuccessful = false;
		}

		return shutdownSuccessful;
	}

	void Application::PushLayer(Layer* const layer)
	{
		CIN_ASSERT(layer, "Invalid layer");
		layer->OnAttach();
		m_LayerStack->PushLayer(layer);
	}

	void Application::PopLayer(Layer* const layer)
	{
		CIN_ASSERT(layer, "Invalid layer");
		layer->OnDetach();
		m_LayerStack->PopLayer(layer);
	}

	void Application::PushOverlay(Layer* const layer)
	{
		CIN_ASSERT(layer, "Invalid layer");
		layer->OnAttach();
		m_LayerStack->PushOverlay(layer);
	}

	void Application::PopOverlay(Layer* const layer)
	{
		CIN_ASSERT(layer, "Invalid layer");
		layer->OnDetach();
		m_LayerStack->PopOverlay(layer);
	}

	const Window* Application::GetWindow() const
	{
		return m_Window;
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
			GraphicsContext::AcquireNextImage();
			GraphicsContext::PresentImage();
		}

		return true;
	}

	bool Application::OnWindowResized(WindowResizedEvent& event)
	{
		const auto [width, height] { event.GetResize() };
		m_Minimized = (width == 0U) or (height == 0U);

		[[likely]]
		if(not m_Minimized)
			GraphicsContext::ResizeSwapchain();
		
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
				m_Window->SetWindowMode(currentWindowMode != EWindowMode::Fullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);

				break;
			}

			default:
			{
				break;
			}
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