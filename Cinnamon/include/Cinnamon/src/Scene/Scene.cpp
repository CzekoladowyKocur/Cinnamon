#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/SceneCamera.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Scene/Environment.hpp"
#include "Cinnamon/include/ECS/Registry.hpp"
#include "Cinnamon/include/Physics/PhysicsWorld2D.hpp"
#include "Cinnamon/include/Physics/PhysicsBody2D.hpp"

namespace Cinnamon {
	Scene::Scene(const ESceneState state) noexcept
		:
		m_Registry(STL::MakeUnique<ECS::Registry>()),
		m_PhysicsWorld(STL::MakeUnique<PhysicsWorld2D>(CinMath::Vector2{ 0.0f, -9.81f })),
		m_Environment(STL::MakeUnique<Environment>()),
		m_DefaultCamera(16.0f / 9.0f),
		m_SceneState(state)
	{
		SetSceneState(m_SceneState);
	}

	Scene::~Scene() noexcept
	{
		for (const ECS::EntityID entityID : ECS::View(m_Registry))
			DestroyEntity(Entity{ entityID, this });
		
		m_PhysicsWorld->Reset();
	}

	void Scene::OnUpdate(const Timestep timestep)
	{
		switch (m_SceneState)
		{
			case ESceneState::Paused:	OnScenePausedUpdate(timestep); break;
			case ESceneState::Playing:	OnScenePlayedUpdate(timestep); break;
			case ESceneState::Edited:	OnSceneEditedUpdate(timestep); break;
			default:					CIN_ASSERT(false); break;
		}
	}

	Entity Scene::CreateEntity(const STL::StringView entityName) noexcept
	{
		Entity entity{ m_Registry->CreateEntity(), this };
		entity.AddComponent<TagComponent>(entityName);
		entity.AddComponent<TransformComponent>();

		return entity;
	}

	void Scene::DestroyEntity(Entity entity) noexcept
	{
		CIN_ASSERT(entity.HasComponent<TagComponent>());

		for (size_t i{ 0U }; i < ECS::GetCurrentMaxComponentID(); ++i)
		{
			/* Call the destructor of each the components. */
			const ECS::ComponentID componentID{ static_cast<ECS::ComponentID>(i) };
			if (m_Registry->Has(entity, componentID))
				ECS::GetComponentDeletion(componentID)(m_Registry->Get(entity, componentID));
		}

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			const RuntimeBodyHandle bodyHandle{ entity.GetComponent<RigidBody2DComponent>().RuntimeBody };
			
			if(bodyHandle)
				m_PhysicsWorld->DestroyBody(entity.GetComponent<RigidBody2DComponent>().RuntimeBody);
		}

		m_Registry->DestroyEntity(entity);
	}

	void Scene::SetSceneState(const ESceneState sceneState)
	{
		switch (sceneState)
		{
			case ESceneState::Paused:	OnScenePause(m_SceneState);	break;
			case ESceneState::Playing:	OnScenePlay(m_SceneState);	break;
			case ESceneState::Edited:	OnSceneEdit(m_SceneState);  break;
			default:					CIN_ASSERT(false); break;
		}

		m_SceneState = sceneState;
	}

	ESceneState Scene::GetSceneState() const
	{
		return m_SceneState;
	}

	void Scene::OnScenePause(const ESceneState /*previousSceneState*/)
	{}

	void Scene::OnScenePlay(const ESceneState previousSceneState)
	{
		/* If scene was pasued, do nothing. Just continue updating it. */
		if (previousSceneState == ESceneState::Paused)
			return;

		/* Reset physics state. */
		m_PhysicsWorld->Reset();
		/* Construct physics bodies. */
		for (const ECS::EntityID entityID : ECS::View<RigidBody2DComponent>(m_Registry))
		{
			auto& transform{ m_Registry->Get<TransformComponent>(entityID) };
			auto& rigidBody{ m_Registry->Get<RigidBody2DComponent>(entityID) };

			PhysicsBody2D physicsBody
			{
				.Type		{ rigidBody.BodyType								},
				.Position	{ transform.Translation.xy	+ rigidBody.Offset		},
				.Angle		{ transform.Rotation.z		+ rigidBody.Angle 		}
			};

			rigidBody.RuntimeBody = m_PhysicsWorld->CreateBody(physicsBody);
			if (m_Registry->Has<Box2DColliderComponent>(entityID))
			{
				auto& box2DCollider{ m_Registry->Get<Box2DColliderComponent>(entityID) };
				const BoxColliderFixture boxColliderFixture{ .Size{ box2DCollider.Size } };

				m_PhysicsWorld->AddFixtureToBody(rigidBody.RuntimeBody, boxColliderFixture);
			}
		}
	}

	void Scene::OnSceneEdit(const ESceneState /*previousSceneState*/)
	{}
	
	void Scene::OnScenePausedUpdate(const Timestep /*timestep*/)
	{}

	void Scene::OnScenePlayedUpdate(const Timestep timestep)
	{
		m_PhysicsWorld->OnUpdate(timestep);

		for (const ECS::EntityID entityID : ECS::View<RigidBody2DComponent>(m_Registry))
		{
			RigidBody2DComponent& rigidBody2D{ m_Registry->Get<RigidBody2DComponent>(entityID) };
			TransformComponent& transform{ m_Registry->Get<TransformComponent>(entityID) };

			transform.Translation.xy	= m_PhysicsWorld->GetBodyPosition(rigidBody2D.RuntimeBody) - rigidBody2D.Offset;
			transform.Rotation.z		= (m_PhysicsWorld->GetBodyAngle(rigidBody2D.RuntimeBody)) + rigidBody2D.Angle;			
		} 
	}

	void Scene::OnSceneEditedUpdate(const Timestep /*timestep*/)
	{}

	STL::Unique<ECS::Registry>& Scene::GetRegistry() noexcept
	{
		return m_Registry;
	}

	const STL::Unique<ECS::Registry>& Scene::GetRegistry() const noexcept
	{
		return m_Registry;
	}

	STL::Unique<Environment>& Scene::GetEnvironment() noexcept
	{
		return m_Environment;
	}

	const STL::Unique<Environment>& Scene::GetEnvironment() const noexcept
	{
		return m_Environment;
	}

	SceneCamera& Scene::GetPrimaryCamera() noexcept
	{
		for (const ECS::EntityID entityID : ECS::View<CameraComponent>(m_Registry))
		{
			auto& sceneCamera{ m_Registry->Get<CameraComponent>(entityID) };

			if (sceneCamera.Primary)
				return { sceneCamera.Camera };
		}

		return m_DefaultCamera;
	}

	const SceneCamera& Scene::GetPrimaryCamera() const noexcept
	{
		for(const ECS::EntityID entityID : ECS::View<CameraComponent>(m_Registry))
		{
			const auto& sceneCamera{ m_Registry->Get<CameraComponent>(entityID) };
			
			if (sceneCamera.Primary)
				return { sceneCamera.Camera };
		}

		return m_DefaultCamera;
	}
}