#include "Cinnamon/include/Core/Application.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"
#include "Cinnamon/include/GUI/GUIRenderer.h"
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Core/LayerStack.h"
#include "Cinnamon/include/Event/Event.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/KeyEvent.h"
#include "Cinnamon/include/Event/MouseEvent.h"
#include "Cinnamon/include/Renderer/Renderer.h"

namespace Cinnamon {
	constinit InternalScope Application* s_ApplicationInstance{ nullptr };

	Application::Application(
		const STL::StringView windowTitle,
		const uint32_t windowWidth,
		const uint32_t windowHeight,
		const bool enableVSync) noexcept
		:
		m_Running(true),
		m_Minimized(false),
		m_LayerStack(STL::MakeUnique<LayerStack>()),
		m_Window(STL::MakeUnique<Window>(WindowProperties{ windowTitle.data(), windowWidth, windowHeight, EWindowMode::Windowed, enableVSync })),
		m_Renderer(STL::MakeUnique<Renderer>(m_Window)),
		m_GUIRenderer(STL::MakeUnique<GUIRenderer>(m_Renderer))
	{
		CIN_ASSERT(s_ApplicationInstance == nullptr, "Application already instantiated!");
		CIN_TRACE("Running cinnamon build {}", Platform::GetBuildDate());

		s_ApplicationInstance = this;
	}

	Application::~Application() noexcept
	{
		s_ApplicationInstance = nullptr;
		CIN_DUMP_ALLOCATIONS();
	}

	Errr Application::Initialize()
	{
		CIN_WARN("Queues from same families might be faster");
		m_Window->SetEventCallback([this](const Event& event)
		{ 
			const EventDispatcher dispatcher(event);
			dispatcher.Dispatch<ApplicationRenderEvent>(std::bind(&Application::OnApplicationRender, this, std::placeholders::_1));
			dispatcher.Dispatch<WindowResizedEvent>(std::bind(&Application::OnWindowResized, this, std::placeholders::_1));
			dispatcher.Dispatch<WindowClosedEvent>(std::bind(&Application::OnWindowClosed, this, std::placeholders::_1));
			dispatcher.Dispatch<KeyPressedEvent>(std::bind(&Application::OnKeyPressed, this, std::placeholders::_1));

			for (Layer* const layer : *m_LayerStack)
				[[likely]] if(not event.IsHandled)
					layer->OnEvent(event);
		});

		if (!OnUserInitialize())
		{
			CIN_CRITICAL("Failed to initialize user data");
			return Error::Failure;
		}

		return Error::Success;
	}

	Errr Application::Run()
	{
		//double lastFrameTime{ Platform::GetAbsoluteTime() };

		[[likely]]
		while (m_Running)
		{
			m_Window->PollEvents();
			OnApplicationRender({});
			//const double currentTime{ Platform::GetAbsoluteTime() };
			//const Timestep timestep{ static_cast<Timestep::Type>(currentTime - lastFrameTime) };
			//lastFrameTime = currentTime;
		}

		return Error::Success;
	}

	void Application::Shutdown()
	{
		OnUserShutdown();
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

	const Window* Application::GetMainWindow() const
	{
		return m_Window.get();
	}

	bool Application::OnApplicationRender(const ApplicationRenderEvent& event)
	{
		FunctionVariable double f_LastFrameTime{ Platform::GetAbsoluteTime() };

		[[likely]]
		if (not m_Minimized)
		{
			const double currentTime{ Platform::GetAbsoluteTime() };
			const Timestep timestep{ static_cast<Timestep::Type>(currentTime - f_LastFrameTime) };
			f_LastFrameTime = currentTime;

			m_Renderer->BeginFrame();
			{
				m_GUIRenderer->BeginFrame();

				for (Layer* const layer : *m_LayerStack)
					layer->OnUpdate(timestep);

				m_GUIRenderer->EndFrame();
			}
			m_Renderer->EndFrame();
		}

		CIN_UNUSED(event);
		return true;
	}

	bool Application::OnWindowResized(const WindowResizedEvent& event)
	{
		const auto [width, height] { event.GetResize() };
		m_Minimized = (width == 0U) or (height == 0U);

		[[likely]]
		if (not m_Minimized)
			m_Renderer->SetViewportSize(width, height);
		
		return true;
	}

	bool Application::OnWindowClosed(const WindowClosedEvent& event)
	{
		CIN_UNUSED(event);
		m_Running = false;

		return true;
	}

	bool Application::OnKeyPressed(const KeyPressedEvent& event)
	{
		const Key key{ event.GetKey() };
		switch(key)
		{
			case Key::F10:
			{
				const EWindowMode currentWindowMode{ m_Window->GetWindowMode() };
				m_Window->SetWindowMode(currentWindowMode != EWindowMode::Maximized ? EWindowMode::Maximized : EWindowMode::Windowed);

				return true;
			}

			case Key::F11:
			{
				const EWindowMode currentWindowMode{ m_Window->GetWindowMode() };
				m_Window->SetWindowMode(currentWindowMode != EWindowMode::Fullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);

				return true;
			}

			default:
			{
				break;
			}
		}

		return false;
	}

	const Application* Application::Get()
	{
		CIN_ASSERT(s_ApplicationInstance, "Application not instantiated");
		return s_ApplicationInstance;
	}

	void Application::Close()
	{
		s_ApplicationInstance->m_Running = false;
	}
}