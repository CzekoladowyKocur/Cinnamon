#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Event;
}

namespace Cinnamon {
	class Layer
	{
	private:
		NON_COPYABLE(Layer)
	public:
		constexpr explicit Layer() noexcept = default;
		constexpr virtual ~Layer() noexcept = default;

		virtual void OnAttach() = 0;
		virtual void OnUpdate(const Timestep timestep) = 0;
		virtual void OnDetach() = 0;

		virtual void OnEvent(const Event& event) = 0;
	private:
	};
}