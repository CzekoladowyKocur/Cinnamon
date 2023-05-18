#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	// ECS is bloated so it has it's own namespace.
	namespace ECS {
		typedef void (*ComponentDeletionFunction)(void* const);
		// Components are designed to be data-only. They shouldn't perform any logic.
		template<typename ComponentType>
		concept ComponentConcept = requires
		{
			std::is_trivially_copyable_v<ComponentType> and std::is_trivially_destructible_v<ComponentType>;
		};

		constexpr size_t g_MaxComponentCount{ 32U };
		using ComponentMask = std::bitset<g_MaxComponentCount>;
		using ComponentID = size_t;
		using EntityID = size_t;
		using EntityIndex = uint32_t;
		using EntityVersion = uint32_t;

		constexpr EntityID Null{ 0xffff'ffff'ffff'ffff };
		
		template<typename Component>
		[[nodiscard]] ComponentID InitializeComponent() noexcept(true)
		{
			extern ComponentID e_ComponentCounter;
			const ComponentID componentID{ e_ComponentCounter++ };

			extern STL::UMap<ComponentID, ComponentDeletionFunction> s_DeletionFunctions;
			s_DeletionFunctions[componentID] = [](void* data)
			{
				Component* const componentData{ reinterpret_cast<Component*>(data) };
				componentData->~Component();
			};

			CIN_TRACE("Initialized component ID of {} for {}", componentID, typeid(Component).name());
			return componentID;
		}

		template<typename Component>
		[[nodiscard]] ComponentID GetComponentID() noexcept(true)
		{
			static ComponentID s_ComponentID{ InitializeComponent<Component>() };
			return s_ComponentID;
		}

		[[nodiscard]] inline ComponentID GetCurrentMaxComponentID() noexcept(true)
		{
			extern ComponentID e_ComponentCounter;
			return e_ComponentCounter;
		}

		[[nodiscard]] inline auto GetComponentDeletion(const ComponentID componentID) noexcept(true)
		{
			extern STL::UMap<ComponentID, ComponentDeletionFunction> s_DeletionFunctions;
			return s_DeletionFunctions[componentID];
		}

		[[nodiscard]] constexpr EntityID CreateEntityID(
			const EntityIndex index,
			const EntityVersion version) noexcept(true)
		{
			return (static_cast<EntityID>(index) << 32U) | (static_cast<EntityID>(version));
		}

		[[nodiscard]] constexpr EntityVersion FetchEntityVersionFromID(const EntityID entityID) noexcept(true)
		{
			return static_cast<EntityVersion>(entityID);
		}

		[[nodiscard]] constexpr EntityVersion FetchEntityIndexFromID(const EntityID entityID) noexcept(true)
		{
			return static_cast<EntityIndex>(entityID >> 32U);
		}

		[[nodiscard]] constexpr bool IsEntityValid(const EntityID entityID) noexcept(true)
		{
			return FetchEntityIndexFromID(entityID) != std::numeric_limits<EntityIndex>::max();
		}
	}
}