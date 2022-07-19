#pragma once
#include "Cinnamon/include/Core/Window.h"

namespace Cinnamon {
	class Application
	{
	private:
	public:
		Application() noexcept;
		~Application() noexcept;

		bool Initialize();
		bool Run();
		bool Shutdown();
	public:
		/* Global application instance */
		static Application* s_ApplicationInstance;
		static const Application* Get();
	protected:
		Window* m_Window;
	private:
	};
}