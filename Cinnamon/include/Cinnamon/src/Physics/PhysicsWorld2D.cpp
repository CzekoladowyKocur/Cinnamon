#include "Cinnamon/include/Physics/PhysicsWorld2D.hpp"
#pragma warning(push, 0)  
#include "box2d/box2d.h"
#include "box2d/b2_contact.h"
#pragma warning(pop)

namespace Cinnamon {
	static_assert(static_cast<uint32_t>(b2_staticBody)		== static_cast<uint32_t>(EBodyType::Static));
	static_assert(static_cast<uint32_t>(b2_kinematicBody)	== static_cast<uint32_t>(EBodyType::Kinematic));
	static_assert(static_cast<uint32_t>(b2_dynamicBody)		== static_cast<uint32_t>(EBodyType::Dynamic));

	class ContactListener final : public b2ContactListener
	{
	private:
	public:
		virtual void BeginContact(b2Contact* const contact) final override
		{
			CIN_UNUSED(contact);
		}

		virtual void EndContact(b2Contact* const contact) final override
		{
			CIN_UNUSED(contact);
		}
	};

	PhysicsWorld2D::PhysicsWorld2D(const CinMath::Vector2& gravity) noexcept
		:
		m_World(STL::MakeUnique<b2World>(b2Vec2{ gravity.x, gravity.y })),
		m_ContactListener(STL::MakeUnique<ContactListener>()),
		m_Gravity(gravity)
	{
		m_World->SetContactListener(m_ContactListener.get());
	}

	PhysicsWorld2D::~PhysicsWorld2D() noexcept
	{}

	void PhysicsWorld2D::OnUpdate(const Timestep timestep)
	{
		m_World->Step(timestep, 8, 4);
	}

	RuntimeBodyHandle PhysicsWorld2D::CreateBody(const PhysicsBody2D& physicsBody)
	{
		b2BodyUserData userData;
		{
			userData.pointer				= reinterpret_cast<uintptr_t>(nullptr);
		};

		b2BodyDef bodyDefinition;
		{
			bodyDefinition.type				= static_cast<b2BodyType>(physicsBody.Type),
			bodyDefinition.position			= { physicsBody.Position.x, physicsBody.Position.y };
			bodyDefinition.angle			= CinMath::ToRadians(physicsBody.Angle);
			bodyDefinition.linearVelocity	= { 0.0f, 0.0f };
			bodyDefinition.angularVelocity	= 0.0f;
			bodyDefinition.linearDamping	= 0.0f;
			bodyDefinition.angularDamping	= 0.0f;
			bodyDefinition.allowSleep		= false;
			bodyDefinition.awake			= true;
			bodyDefinition.fixedRotation	= false;
			bodyDefinition.bullet			= false;
			bodyDefinition.enabled			= true;
			bodyDefinition.userData			= userData;
			bodyDefinition.gravityScale		= 1.0f;
		}

		return m_World->CreateBody(&bodyDefinition);
	}

	void PhysicsWorld2D::DestroyBody(RuntimeBodyHandle const body)
	{
		CIN_ASSERT(body);
		m_World->DestroyBody(body);
	}

	void PhysicsWorld2D::AddFixtureToBody(const RuntimeBodyHandle body, const BoxColliderFixture& fixture)
	{
		b2PolygonShape polygonShape;
		polygonShape.SetAsBox(fixture.Size.x * 0.5f, fixture.Size.y * 0.5f);

		b2FixtureDef b2FixtureDefinition;
		{
			b2FixtureDefinition.shape					= &polygonShape;
			b2FixtureDefinition.userData				= {};
			b2FixtureDefinition.friction				= 0.5f;
			b2FixtureDefinition.restitution				= 0.0f;
			b2FixtureDefinition.restitutionThreshold	= 1.0f;
			b2FixtureDefinition.density					= 1.0f;
			b2FixtureDefinition.isSensor				= false;
			b2FixtureDefinition.filter					= {};
		}

		body->CreateFixture(&b2FixtureDefinition);
	}

	CinMath::Vector2 PhysicsWorld2D::GetBodyPosition(const RuntimeBodyHandle body) const
	{
		CIN_ASSERT(body);
		return CinMath::Vector2{ body->GetPosition().x, body->GetPosition().y };
	}

	float PhysicsWorld2D::GetBodyAngle(const RuntimeBodyHandle body) const
	{
		return -CinMath::ToDegrees(body->GetAngle());
	}

	CinMath::Vector2 PhysicsWorld2D::GetGravity() const
	{
		return m_Gravity;
	}

	void PhysicsWorld2D::Reset()
	{
		m_World = STL::MakeUnique<b2World>(b2Vec2{ m_Gravity.x, m_Gravity.y });
	}
}