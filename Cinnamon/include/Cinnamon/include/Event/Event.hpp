#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	/* Event types */
	class Event;
	using EventCallbackFunction = std::function<void(const Event& event)>;
#define BIND_EVENT_FUNCTION(function) std::bind(&##function, this, std::placeholders::_1)
#define EVENT_TYPE(event_) \
	static consteval EEventType GetEventTypeStatic() { static_assert(std::is_base_of<Event, event_##Event>::value, "Class is not derived from Event!"); return EEventType::event_; } \
	CIN_FORCE_INLINE EEventType GetEventType() const override final { return GetEventTypeStatic(); }

#define EVENT_CATEGORY(event_) \
	CIN_FORCE_INLINE EEventCategory GetEventCategory() const override final { return EEventCategory::event_; }

	template<typename T>
	concept EventClassDerivative = std::is_base_of<Event, T>::value;
}

namespace Cinnamon {
	enum class EEventType
	{
		ApplicationTick, ApplicationRender,
		WindowClosed, WindowResized, WindowMinimized, WindowMaximized, WindowSurfaceUpdated,
		KeyPressed, KeyHeld, KeyReleased,
		MousePressed, MouseHeld, MouseReleased, MouseMoved, MouseScrolled
	};

	enum class EEventCategory
	{
		Application		= BIT(1),
		WindowSurface	= BIT(2),
		Input			= BIT(3),
		Keyboard		= BIT(4) | Input,
		Mouse			= BIT(5) | Input,
	};

	class Event
	{
	public:
		constexpr Event() noexcept = default;
		constexpr virtual ~Event() noexcept = default;

		virtual EEventType GetEventType() const = 0;
		virtual EEventCategory GetEventCategory() const = 0;
	public:
		mutable bool IsHandled{ false };
	};

	class EventDispatcher
	{
	private:
	public:
		explicit EventDispatcher(const Event& event) noexcept;

		template<EventClassDerivative EventType, typename DispatchFunction>
		CIN_FORCE_INLINE void Dispatch(const DispatchFunction&& function) const noexcept
		{
			if (m_Event.GetEventType() == ResolveAtCompileTime(EventType::GetEventTypeStatic()))
				m_Event.IsHandled |= function(static_cast<const EventType&>(m_Event));
		}
	private:
		const Event& m_Event;
	};
}