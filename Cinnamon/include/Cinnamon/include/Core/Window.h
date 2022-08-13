#pragma once
#include "Cinnamon/include/Core/Core.h"
#include "Cinnamon/include/Event/Event.h"

namespace Cinnamon {
	struct PlatformWindowState;

	enum class EWindowMode
	{
		Unspecified = 0,
		Windowed = 1,
		WindowedFullscreen = 2,
		Maximized = 3
	};

	struct WindowProperties
	{
		/* TODO: Change to string class */
		const char8_t* Name;
		uint32_t Width;
		uint32_t Height;
		EWindowMode Mode;
		/* TODO: Add vsync */

		explicit WindowProperties(
			const char8_t* windowName,
			const uint32_t windowWidth,
			const uint32_t windowHeight,
			const EWindowMode windowMode) noexcept
			:
			Name(windowName),
			Width(windowWidth),
			Height(windowHeight),
			Mode(windowMode)
		{}

		~WindowProperties() = default;
	};

	class Window
	{
	private:
	public:
		explicit Window(WindowProperties&& windowProperties, const EventCallbackFunction callback = nullptr) noexcept;
		~Window() noexcept;

		void PollEvents();
		void SendEvent(Event& event);

		const char8_t* GetName() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		EWindowMode GetWindowMode() const;
		std::pair<uint32_t, uint32_t> GetSize() const;
		EventCallbackFunction GetEventCallback() const;
		const PlatformWindowState* GetState() const;
		const void* GetNativeHandle() const;

		void SetName(const char8_t* windowName);
		void SetWidth(const uint32_t windowWidth);
		void SetHeight(const uint32_t windowHeight);
		void SetWindowMode(const EWindowMode windowMode);
		void SetSize(std::pair<uint32_t, uint32_t> windowSize);
		void SetEventCallback(const EventCallbackFunction callback);
	private:
		/* To be defined in platform */
		PlatformWindowState* m_State;
		WindowProperties m_Properties;
		EventCallbackFunction m_EventCallback;
	};
}