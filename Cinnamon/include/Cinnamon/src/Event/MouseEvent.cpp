#include "Cinnamon/include/Event/MouseEvent.hpp"

namespace Cinnamon {
	MouseEvent::MouseEvent(const MouseCode mouseCode) noexcept
		:
		m_MouseCode(mouseCode)
	{}

	MouseCode MouseEvent::GetMouseCode() const
	{
		return m_MouseCode;
	}

	MousePressedEvent::MousePressedEvent(const MouseCode mouseCode) noexcept
		:
		MouseEvent(mouseCode)
	{}

	MouseReleasedEvent::MouseReleasedEvent(const MouseCode mouseCode) noexcept
		:
		MouseEvent(mouseCode)
	{}

	MouseMovedEvent::MouseMovedEvent(const uint32_t positionX, const uint32_t positionY) noexcept
		:
		m_PositionX(positionX),
		m_PositionY(positionY)
	{}

	std::pair<double, double> MouseMovedEvent::GetPosition() const
	{
		return { m_PositionX, m_PositionY };
	}

	MouseScrolledEvent::MouseScrolledEvent(const uint16_t horizontalDelta, const uint16_t verticalDelta) noexcept
		:
		m_HorizontalDelta(horizontalDelta),
		m_VerticalDelta(verticalDelta)
	{}

	uint16_t MouseScrolledEvent::GetHorizontalDelta() const
	{
		return m_HorizontalDelta;
	}

	uint16_t MouseScrolledEvent::GetVerticalDelta() const
	{
		return m_VerticalDelta;
	}

	std::pair<uint16_t, uint16_t> MouseScrolledEvent::GetDelta() const
	{
		return { m_HorizontalDelta, m_VerticalDelta };
	}
}