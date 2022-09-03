#pragma once
#include "Cinnamon/include/Core/Core.h"
#include "Cinnamon/include/Event/Event.h"

namespace Cinnamon {
	struct PlatformWindowState;
	class InputState;
}

namespace Cinnamon {
	enum class EWindowMode
	{
		Unspecified = 0,
		Windowed = 1,
		Maximized = 2,
		Fullscreen = 3,
	};

	struct WindowProperties
	{
		const char* Name;
		uint32_t Width;
		uint32_t Height;
		EWindowMode Mode;
		bool UseVSync;
		bool Focused;

		explicit WindowProperties(
			const char* windowName,
			const uint32_t windowWidth,
			const uint32_t windowHeight,
			const EWindowMode windowMode,
			const bool useVSync) noexcept
			:
			Name(windowName),
			Width(windowWidth),
			Height(windowHeight),
			Mode(windowMode),
			UseVSync(useVSync),
			Focused(false)
		{}

		constexpr ~WindowProperties() noexcept = default;
	};

	class Window
	{
	private:
		NON_COPYABLE(Window)
	public:
		explicit Window(WindowProperties&& windowProperties, const EventCallbackFunction callback = nullptr) noexcept;
		~Window() noexcept;

		void PollEvents();
		void SendEvent(Event& event);

		const char* GetName() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		EWindowMode GetWindowMode() const;
		std::pair<uint32_t, uint32_t> GetSize() const;
		EventCallbackFunction GetEventCallback() const;
		const PlatformWindowState* GetState() const;
		const InputState* GetInputState() const;
		WindowProperties& GetProperties();
		const WindowProperties& GetProperties() const;
		const void* GetNativeHandle() const;

		void SetName(const char* windowName);
		void SetWidth(const uint32_t windowWidth);
		void SetHeight(const uint32_t windowHeight);
		void SetWindowMode(const EWindowMode windowMode);
		void SetSize(const std::pair<uint32_t, uint32_t> windowSize);
		void SetEventCallback(const EventCallbackFunction callback);
	private:
		/* To be defined in platform */
		PlatformWindowState* m_State;
		InputState* m_InputState;
		WindowProperties m_Properties;
		EventCallbackFunction m_EventCallback;

#ifdef CIN_PLATFORM_WINDOWS
		friend InternalScope LRESULT CALLBACK Windows32ProcessMessage(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam);
#endif
	};
}