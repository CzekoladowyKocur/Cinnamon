#pragma once
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Event/Event.h"
#include "Cinnamon/include/Event/ApplicationEvent.h"
#include "Cinnamon/include/Event/WindowEvent.h"
#include "Cinnamon/include/Event/KeyEvent.h"
#include "Cinnamon/include/Event/MouseEvent.h"

namespace Cinnamon {
	class Application
	{
	private:
	public:
		Application() noexcept;
		~Application() noexcept;

		[[nodiscard]] bool Initialize();
		[[nodiscard]] bool Run();
		[[nodiscard]] bool Shutdown();
	private:
		void OnEvent(Event& event);

		bool OnApplicationRender(ApplicationRenderEvent& event);
		bool OnWindowResized(WindowResizedEvent& event);
		bool OnWindowClosed(WindowClosedEvent& event);
		bool OnKeyPressed(KeyPressedEvent& event);
	public:
		/* Global application instance */
		static Application* s_ApplicationInstance;
		static const Application* Get();
	protected:
		bool m_Running;
		bool m_Minimized;

		Window* m_Window;
	private:
	};
}