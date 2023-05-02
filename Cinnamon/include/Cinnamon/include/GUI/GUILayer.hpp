#pragma once
#include "Cinnamon/include/Core/Layer.h"

namespace Cinnamon {
	class GUILayer final : public Layer
	{
	private:
		NON_COPYABLE(GUILayer)
	public:
		consteval explicit GUILayer() noexcept = default;
		constexpr virtual ~GUILayer() noexcept = default;
	};
}