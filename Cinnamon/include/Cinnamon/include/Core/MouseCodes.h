#pragma once
#include "Cinnamon/include/Core/Core.h"

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
		LeftButton,
		MiddleButton,
		RightButton,
#ifdef CIN_PLATFORM_WINDOWS
#elif defined CIN_PLATFORM_LINUX
#elif defined CIN_PLATFORM_APPLE
#endif
		MouseButtonsEnd,
	};

	CIN_FORCE_INLINE MouseStateEnumType operator&(const EMouseState lhs, const EMouseState rhs) noexcept
	{
		return static_cast<MouseStateEnumType>(lhs) & static_cast<MouseStateEnumType>(rhs);
	}
}