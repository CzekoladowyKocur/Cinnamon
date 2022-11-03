#include "Cinnamon/include/Event/Event.h"

namespace Cinnamon {
	EventDispatcher::EventDispatcher(const Event& event) noexcept
		:
		m_Event(event)
	{}
}