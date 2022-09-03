#pragma once
#include "Cinnamon/include/Core/Core.h"
#include "Cinnamon/include/Core/KeyCodes.h"
#include "Cinnamon/include/Core/MouseCodes.h"

namespace Cinnamon {
	class InputState
	{
	private:
	public:
		explicit InputState() noexcept;
		constexpr ~InputState() noexcept = default;

		void SetKeyState(const Key key, const EKeyState keyState);
		void SetMouseButtonState(const Mouse mouseButton, const EMouseState mouseState);
		void SetMouseCursorPosition(const uint32_t xPosition, const uint32_t yPosition);

		EKeyState GetKeyState(const Key key) const;
		EMouseState GetMouseButtonState(const Mouse mouseButton) const;
		std::pair<uint32_t, uint32_t> GetMousePosition() const;
	private:
		EKeyState m_KeyStates[static_cast<const std::size_t>(Key::KeysEnd)];
		EMouseState m_MouseStates[static_cast<const std::size_t>(Mouse::MouseButtonsEnd)];

		struct
		{
			uint32_t x, y;
		} m_MousePosition;
	};
	
	class Input
	{
	private:
		NON_CONSTRUCTIBLE(Input)
		NON_COPYABLE(Input)
	public:
		static bool IsKeyPressed(const Key key);
		static bool IsMouseButtonPressed(const Mouse mouseButton);
		static std::pair<uint32_t, uint32_t> GetMousePosition();
	private:
	};
}