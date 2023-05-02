#pragma once
#include "Cinnamon/include/Event/Event.hpp"
#include "Cinnamon/include/Core/KeyCodes.hpp"

namespace Cinnamon {
	class KeyEvent : public Event
	{
	private:
	public:
		explicit KeyEvent(const KeyCode keyCode) noexcept;
		constexpr virtual ~KeyEvent() noexcept = default;

		virtual KeyCode GetKeyCode() const noexcept final;
		virtual Key GetKey() const noexcept final;
	protected:
		const KeyCode m_KeyCode;
	};

	class KeyPressedEvent final : public KeyEvent
	{
	private:
	public:
		explicit KeyPressedEvent(const KeyCode keyCode) noexcept;
		constexpr virtual ~KeyPressedEvent() noexcept = default;

		EVENT_TYPE(KeyPressed)
		EVENT_CATEGORY(Keyboard)
	private:
	};
	
	class KeyReleasedEvent final : public KeyEvent
	{
	private:
	public:
		explicit KeyReleasedEvent(const KeyCode keyCode) noexcept;
		constexpr virtual ~KeyReleasedEvent() noexcept = default;

		EVENT_TYPE(KeyReleased)
		EVENT_CATEGORY(Keyboard)
	private:
	};

	class KeyHeldEvent final : public KeyEvent
	{
	private:
	public:
		explicit KeyHeldEvent(const KeyCode keyCode) noexcept;
		constexpr virtual ~KeyHeldEvent() noexcept = default;

		EVENT_TYPE(KeyHeld)
		EVENT_CATEGORY(Keyboard)
	private:
	};
}