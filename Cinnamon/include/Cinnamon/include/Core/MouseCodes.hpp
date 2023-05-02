#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	using MouseCode = uint16_t;
	using MouseStateEnumType = uint8_t;

	enum class EMouseState : MouseStateEnumType
	{
		Released	= BIT(1),
		Pressed		= BIT(2),
		Repeat		= BIT(3)
	};

	enum class Mouse : MouseCode
	{
#ifdef CIN_PLATFORM_WINDOWS
		LeftButton,
		MiddleButton,
		RightButton,
#elif defined CIN_PLATFORM_LINUX
		LeftButton = 272,
		MiddleButton = 274,
		RightButton = 273,
#elif defined CIN_PLATFORM_APPLE
#endif
		MouseButtonsEnd,
	};

	constexpr MouseStateEnumType operator&(const EMouseState lhs, const EMouseState rhs) noexcept
	{
		return static_cast<MouseStateEnumType>(lhs) & static_cast<MouseStateEnumType>(rhs);
	}
}