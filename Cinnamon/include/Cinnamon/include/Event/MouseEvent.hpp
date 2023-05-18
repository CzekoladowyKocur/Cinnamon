#pragma once
#include "Cinnamon/include/Event/Event.hpp"
#include "Cinnamon/include/Core/MouseCodes.hpp"

namespace Cinnamon {
	class MouseEvent : public Event
	{
	private:
	public:
		explicit MouseEvent(const MouseCode mouseCode) noexcept;
		constexpr virtual ~MouseEvent() noexcept = default;

		virtual MouseCode GetMouseCode() const final;
		virtual Mouse GetMouseButton() const final;
	private:
		const MouseCode m_MouseCode;
	};

	class MousePressedEvent final : public MouseEvent
	{
	private:
	public:
		explicit MousePressedEvent(const MouseCode mouseCode) noexcept;
		constexpr virtual ~MousePressedEvent() noexcept = default;

		EVENT_TYPE(MousePressed)
		EVENT_CATEGORY(Mouse)
	private:
	};

	class MouseReleasedEvent final : public MouseEvent
	{
	private:
	public:
		explicit MouseReleasedEvent(const MouseCode mouseCode) noexcept;
		constexpr virtual ~MouseReleasedEvent() noexcept = default;

		EVENT_TYPE(MouseReleased)
		EVENT_CATEGORY(Mouse)
	private:
	};

	class MouseMovedEvent final : public Event
	{
	private:
	public:
		explicit MouseMovedEvent(const uint32_t positionX, const uint32_t positionY) noexcept;
		constexpr virtual ~MouseMovedEvent() noexcept = default;

		std::pair<double, double> GetPosition() const;

		EVENT_TYPE(MouseMoved)
		EVENT_CATEGORY(Mouse)
	private:
		const uint32_t m_PositionX, m_PositionY;
	};

	class MouseScrolledEvent final : public Event
	{
	private:
	public:
		MouseScrolledEvent(const int16_t horizontalDelta, const int16_t verticalDelta) noexcept;
		constexpr virtual ~MouseScrolledEvent() noexcept = default;

		int16_t GetHorizontalDelta() const;
		int16_t GetVerticalDelta() const;
		std::pair<int16_t, int16_t> GetDelta() const;

		EVENT_TYPE(MouseScrolled)
		EVENT_CATEGORY(Mouse)
	private:
		const int16_t m_HorizontalDelta, m_VerticalDelta;
	};
}