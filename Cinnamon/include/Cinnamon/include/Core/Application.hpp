#pragma once
#include "Cinnamon/include/Core/Core.hpp"

#define REGISTER_CINNAMON_APPLICATION(applicationName)						\
Cinnamon::Application* Cinnamon::CreateApplication() noexcept				\
{																			\
	using namespace Cinnamon;												\
	static_assert(std::is_base_of<Application, applicationName>::value);	\
	return cinew applicationName();											\
}							

namespace Cinnamon {
	class Layer;
	class LayerStack;
	class Window;
	class Renderer;
	class GUIRenderer;
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
		explicit Application(
			const STL::StringView windowTitle, 
			const uint32_t windowWidth, 
			const uint32_t windowHeight, 
			const bool enableVSync) noexcept;
		
		virtual ~Application() noexcept;

		Errr Initialize();
		Errr Run();
		void Shutdown();
		
		void PushLayer(Layer* const layer);
		void PopLayer(Layer* const layer);
		void PushOverlay(Layer* const layer);
		void PopOverlay(Layer* const layer);

		[[nodiscard]] const Window* GetMainWindow() const;
	protected: 
		/* User functions */
		virtual Errr OnUserInitialize() = 0;
		virtual void OnUserShutdown() = 0;
	private:
		bool OnApplicationRender(const ApplicationRenderEvent& event);
		bool OnWindowResized(const WindowResizedEvent& event);
		bool OnWindowClosed(const WindowClosedEvent& event);
		bool OnKeyPressed(const KeyPressedEvent& event);
	protected:
		bool m_Running;
		bool m_Minimized;

		STL::Unique<LayerStack> m_LayerStack;
		STL::Unique<Window> m_MainWindow;
		STL::Unique<Renderer> m_Renderer;
		STL::Unique<GUIRenderer> m_GUIRenderer;
	public:
		static const Application* Get();
		static void Close();
	};

	Application* CreateApplication() noexcept;
}