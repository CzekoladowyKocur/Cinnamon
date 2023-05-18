#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/SceneCamera.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Scene/Environment.hpp"
#include "Cinnamon/include/ECS/Registry.hpp"

namespace Cinnamon {
	Scene::Scene() noexcept
		:
		m_Registry(STL::MakeUnique<ECS::Registry>()),
		m_Environment(STL::MakeUnique<Environment>()),
		m_DefaultCamera(16.0f / 9.0f)
	{}

	Scene::~Scene() noexcept
	{
		for (const ECS::EntityID entityID : ECS::View(m_Registry))
			DestroyEntity(Entity{ entityID, this });
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

		m_Registry->DestroyEntity(entity);
	}

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
}