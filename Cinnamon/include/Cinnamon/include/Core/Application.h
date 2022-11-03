#pragma once
#include "Cinnamon/include/Core/Core.h"

#define REGISTER_CINNAMON_APPLICATION(applicationName)						\
Cinnamon::Application* Cinnamon::CreateApplication() noexcept				\
{																			\
	using namespace Cinnamon;												\
	static_assert(std::is_base_of<Application, applicationName>::value);	\
	return cinew applicationName();											\
}							

namespace Cinnamon {
	class Window;
	class Layer;
	class LayerStack;
	class Event;
	class ApplicationRenderEvent;
	class WindowResizedEvent;
	class WindowClosedEvent;
	class KeyPressedEvent;
}

namespace Cinnamon {
	class Application
	{
	private:
		NON_COPYABLE(Application)
	public:
		Application() noexcept;
		virtual ~Application() noexcept;

		[[nodiscard]] bool Initialize();
		[[nodiscard]] bool Run();
		[[nodiscard]] bool Shutdown();
		
		void PushLayer(Layer* const layer);
		void PopLayer(Layer* const layer);
		void PushOverlay(Layer* const layer);
		void PopOverlay(Layer* const layer);

		[[nodiscard]] const Window* GetWindow() const;
	protected: 
		/* User functions */
		[[nodiscard]] virtual bool OnUserInitialize() = 0;
		[[nodiscard]] virtual bool OnUserShutdown() = 0;
	private:
		bool OnApplicationRender(const ApplicationRenderEvent& event);
		bool OnWindowResized(const WindowResizedEvent& event);
		bool OnWindowClosed(const WindowClosedEvent& event);
		bool OnKeyPressed(const KeyPressedEvent& event);
	protected:
		mutable bool m_Running;
		bool m_Minimized;

		Window* m_Window;
		LayerStack* m_LayerStack;
	public:
		static const Application* Get();
		static void Close();
	};

	Application* CreateApplication() noexcept;
}