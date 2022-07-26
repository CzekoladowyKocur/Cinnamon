#include "Cinnamon/include/Event/KeyEvent.h"
#include "Cinnamon/include/Core/KeyCodes.h"

namespace Cinnamon {
	KeyEvent::KeyEvent(const KeyCode keyCode) noexcept
		:
		m_KeyCode(keyCode)
	{}

	KeyCode KeyEvent::GetKeyCode() const noexcept
	{
		return m_KeyCode;
	}

	Key KeyEvent::GetKey() const noexcept
	{
		return static_cast<Key>(m_KeyCode);
	}

	KeyPressedEvent::KeyPressedEvent(const KeyCode keyCode) noexcept
		:
		KeyEvent(keyCode)
	{}

	KeyReleasedEvent::KeyReleasedEvent(const KeyCode keyCode) noexcept
		:
		KeyEvent(keyCode)
	{}

	KeyHeldEvent::KeyHeldEvent(const KeyCode keyCode) noexcept
		:
		KeyEvent(keyCode)
	{}
}