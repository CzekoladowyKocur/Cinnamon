#include "Cinnamon/include/Event/Event.hpp"

namespace Cinnamon {
	EventDispatcher::EventDispatcher(const Event& event) noexcept
		:
		m_Event(event)
	{}
}