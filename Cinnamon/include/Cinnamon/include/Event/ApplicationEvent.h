#pragma once
#include "Cinnamon/include/Event/Event.h"

namespace Cinnamon {
	class ApplicationTickEvent final : public Event
	{
	private:
	public:
		ApplicationTickEvent() noexcept = default;
		virtual ~ApplicationTickEvent() noexcept = default;

		EVENT_TYPE(ApplicationTick)
		EVENT_CATEGORY(Application)
	};

	class ApplicationRenderEvent final : public Event
	{
	private:
	public:
		ApplicationRenderEvent() noexcept = default;
		virtual ~ApplicationRenderEvent() noexcept = default;

		EVENT_TYPE(ApplicationRender)
		EVENT_CATEGORY(Application)
	private:
	};
}