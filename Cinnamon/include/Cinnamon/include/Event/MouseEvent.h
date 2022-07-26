#pragma once
#include "Cinnamon/include/Event/Event.h"
#include "Cinnamon/include/Core/MouseCodes.h"

namespace Cinnamon {
	class MouseEvent : public Event
	{
	private:
	public:
		explicit MouseEvent(const MouseCode mouseCode) noexcept;
		virtual ~MouseEvent() noexcept = default;

		virtual MouseCode GetMouseCode() const final;
	private:
		const MouseCode m_MouseCode;
	};

	class MousePressedEvent final : public MouseEvent
	{
	private:
	public:
		explicit MousePressedEvent(const MouseCode mouseCode) noexcept;
		virtual ~MousePressedEvent() noexcept = default;
	private:
	};

	class MouseReleasedEvent final : public MouseEvent
	{
	private:
	public:
		explicit MouseReleasedEvent(const MouseCode mouseCode) noexcept;
		virtual ~MouseReleasedEvent() noexcept = default;
	private:
	};

	class MouseScrolledEvent final : public Event
	{
	private:
	public:
		MouseScrolledEvent(const uint16_t horizontalDelta, const uint16_t verticalDelta) noexcept;
		virtual ~MouseScrolledEvent() noexcept = default;

		uint16_t GetHorizontalDelta() const;
		uint16_t GetVerticalDelta() const;
		std::pair<uint16_t, uint16_t> GetDelta() const;
	private:
		const uint16_t m_HorizontalDelta, m_VerticalDelta;
	};
}