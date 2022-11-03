#include "Cinnamon/include/Core/Input.h"
#include "Cinnamon/include/Core/Application.h"
#include "Cinnamon/include/Core/Window.h"

namespace Cinnamon {
	InputState::InputState() noexcept
	{
		std::memset(m_KeyStates, static_cast<int>(EKeyState::Released), CIN_CARRAY_SIZE(m_KeyStates));
		std::memset(m_MouseStates, static_cast<int>(EMouseState::Released), CIN_CARRAY_SIZE(m_MouseStates));
		m_MousePosition.x = 0U;
		m_MousePosition.y = 0U;
	}

	void InputState::SetKeyState(const Key key, const EKeyState keyState)
	{
		m_KeyStates[static_cast<std::size_t>(key)] = keyState;
	}

	void InputState::SetMouseButtonState(const Mouse mouseButton, const EMouseState mouseState)
	{
		m_MouseStates[static_cast<std::size_t>(mouseButton)] = mouseState;
	}

	void InputState::SetMouseCursorPosition(const uint32_t xPosition, const uint32_t yPosition)
	{
		m_MousePosition.x = xPosition;
		m_MousePosition.y = yPosition;
	}

	EKeyState InputState::GetKeyState(const Key key) const
	{
		return m_KeyStates[static_cast<std::size_t>(key)];
	}

	EMouseState InputState::GetMouseButtonState(const Mouse mouseButton) const
	{
		return m_MouseStates[static_cast<std::size_t>(mouseButton)];
	}

	std::pair<uint32_t, uint32_t> InputState::GetMousePosition() const
	{
		return { m_MousePosition.x, m_MousePosition.y };
	}

	bool Input::IsKeyPressed(const Key key)
	{
		const Application* application{ Application::Get() };
		return application->GetWindow()->GetInputState()->GetKeyState(key) & EKeyState::Pressed;
	}

	bool Input::IsMouseButtonPressed(const Mouse mouseButton)
	{
		const Application* application{ Application::Get() };
		return application->GetWindow()->GetInputState()->GetMouseButtonState(mouseButton) & EMouseState::Pressed;
	}

	std::pair<uint32_t, uint32_t> Input::GetMousePosition()
	{
		const Application * application{ Application::Get() };
		return application->GetWindow()->GetInputState()->GetMousePosition();
	}
}