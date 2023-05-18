#include "Cinnamon/include/Scene/Entity.hpp"
#include "Cinnamon/include/ECS/Registry.hpp"
#include "Cinnamon/include/Scene/Scene.hpp"

namespace Cinnamon {
	void* Entity::AddComponentInternal(const ECS::ComponentID componentID, const size_t componentSize) noexcept
	{
		CIN_ASSERT(m_Scene);
		CIN_ASSERT(ECS::IsEntityValid(m_EntityID));
		CIN_ASSERT(not HasComponentInternal(componentID));

		return m_Scene->m_Registry->Assign(m_EntityID, componentID, componentSize);
	}

	void* Entity::GetComponentInternal(const ECS::ComponentID componentID) noexcept
	{
		CIN_ASSERT(m_Scene);
		CIN_ASSERT(ECS::IsEntityValid(m_EntityID));
		CIN_ASSERT(HasComponentInternal(componentID));
		
		return m_Scene->m_Registry->Get(m_EntityID, componentID);
	}

	void* Entity::GetComponentInternal(const ECS::ComponentID componentID) const noexcept
	{
		CIN_ASSERT(m_Scene);
		CIN_ASSERT(ECS::IsEntityValid(m_EntityID));
		CIN_ASSERT(HasComponentInternal(componentID));

		return m_Scene->m_Registry->Get(m_EntityID, componentID);
	}

	bool Entity::HasComponentInternal(const ECS::ComponentID componentID) const noexcept
	{
		CIN_ASSERT(m_Scene);
		CIN_ASSERT(ECS::IsEntityValid(m_EntityID));

		return m_Scene->m_Registry->Has(m_EntityID, componentID);
	}

	void Entity::RemoveComponentInternal(const ECS::ComponentID componentID) noexcept
	{
		CIN_ASSERT(m_Scene);
		CIN_ASSERT(ECS::IsEntityValid(m_EntityID));
		CIN_ASSERT(HasComponentInternal(componentID));

		m_Scene->m_Registry->Remove(m_EntityID, componentID);
	}
}