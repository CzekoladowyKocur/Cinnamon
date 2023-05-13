#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/ECS/Registry.hpp"

#define REMOVE_ENTITY_COMPONENT_IF_HAS(entity, T) if(entity.HasComponent<T>()) { entity.RemoveComponent<T>(); }

namespace Cinnamon {
	Scene::Scene() noexcept
		:
		m_Registry(STL::MakeUnique<ECS::Registry>())
	{}

	Scene::~Scene() noexcept
	{
		for (const ECS::EntityID entityID : ECS::View(m_Registry))
			DestroyEntity(Entity{ entityID, this });
	}

	[[nodiscard]] Entity Scene::CreateEntity(const STL::StringView entityName) noexcept
	{
		Entity entity{ m_Registry->CreateEntity(), this };
		entity.AddComponent<TagComponent>(entityName);
		entity.AddComponent<TransformComponent>();

		return entity;
	}

	void Scene::DestroyEntity(Entity entity) noexcept
	{
		CIN_ASSERT(entity.HasComponent<TagComponent>());
		entity.RemoveComponent<TagComponent>();

		m_Registry->DestroyEntity(entity);
	}

	[[nodiscard]] STL::Unique<ECS::Registry>& Scene::GetRegistry() noexcept
	{
		return m_Registry;
	}

	const STL::Unique<ECS::Registry>& Scene::GetRegistry() const noexcept
	{
		return m_Registry;
	}
}