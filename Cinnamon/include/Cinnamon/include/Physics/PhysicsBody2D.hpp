#pragma once
#include "CinMath/CinMath.h"

class b2Body;

namespace Cinnamon {
	using RuntimeBodyHandle = b2Body*;

	enum class EBodyType
	{
		Static		= 0,
		Kinematic	= 1,
		Dynamic		= 2
	};

	struct PhysicsBody2D final
	{
		EBodyType			Type{ EBodyType::Static };
		CinMath::Vector2	Position{ 0.0f };
		float				Angle{ 0.0f };
	};

	struct BoxColliderFixture final
	{
		CinMath::Vector2	Size;
	};
}