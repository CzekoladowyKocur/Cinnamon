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

	Mouse MouseEvent::GetMouseButton() const
	{
		return static_cast<Mouse>(m_MouseCode);
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

	MouseScrolledEvent::MouseScrolledEvent(const int16_t horizontalDelta, const int16_t verticalDelta) noexcept
		:
		m_HorizontalDelta(horizontalDelta),
		m_VerticalDelta(verticalDelta)
	{}

	int16_t MouseScrolledEvent::GetHorizontalDelta() const
	{
		return m_HorizontalDelta;
	}

	int16_t MouseScrolledEvent::GetVerticalDelta() const
	{
		return m_VerticalDelta;
	}

	std::pair<int16_t, int16_t> MouseScrolledEvent::GetDelta() const
	{
		return { m_HorizontalDelta, m_VerticalDelta };
	}
}