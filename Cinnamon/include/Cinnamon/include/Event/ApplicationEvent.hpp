#pragma once
#include "Cinnamon/include/Event/Event.hpp"

namespace Cinnamon {
	class ApplicationTickEvent final : public Event
	{
	private:
	public:
		constexpr ApplicationTickEvent() noexcept = default;
		constexpr virtual ~ApplicationTickEvent() noexcept = default;

		EVENT_TYPE(ApplicationTick)
		EVENT_CATEGORY(Application)
	};

	class ApplicationRenderEvent final : public Event
	{
	private:
	public:
		constexpr ApplicationRenderEvent() noexcept = default;
		constexpr virtual ~ApplicationRenderEvent() noexcept = default;

		EVENT_TYPE(ApplicationRender)
		EVENT_CATEGORY(Application)
	private:
	};
}