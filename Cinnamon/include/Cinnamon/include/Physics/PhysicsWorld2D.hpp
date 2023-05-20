#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Physics/PhysicsBody2D.hpp"
#include "CinMath/CinMath.h"

class b2World;
namespace Cinnamon {
	class ContactListener;

	class PhysicsWorld2D final
	{
	public:
		explicit PhysicsWorld2D(const CinMath::Vector2& gravity) noexcept;
		~PhysicsWorld2D() noexcept;

		void OnUpdate(const Timestep timestep);

		[[nodiscard]] RuntimeBodyHandle 
			CreateBody(const PhysicsBody2D& physicsBody);
		
		void DestroyBody(RuntimeBodyHandle const body);

		void AddFixtureToBody(const RuntimeBodyHandle body, const BoxColliderFixture& fixture);

		[[nodiscard]] CinMath::Vector2
			GetBodyPosition(const RuntimeBodyHandle body) const;

		[[nodiscard]] float
			GetBodyAngle(const RuntimeBodyHandle body) const;

		[[nodiscard]] CinMath::Vector2
			GetGravity() const;

		void Reset();
	private:
		STL::Unique<b2World>			m_World;
		STL::Unique<ContactListener>	m_ContactListener;
		CinMath::Vector2				m_Gravity;
	};
}