#include "Cinnamon/include/Event/Event.h"

namespace Cinnamon {
	EventDispatcher::EventDispatcher(Event& event) noexcept
		:
		m_Event(event)
	{}
}