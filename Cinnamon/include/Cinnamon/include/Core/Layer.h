#pragma once
#include "Cinnamon/include/Core/Core.h"

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
	private:
	};
}