#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	// ECS is bloated so it has it's own namespace.
	namespace ECS {
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
		[[nodiscard]] ComponentID GetComponentID() noexcept(true)
		{
			extern ComponentID e_ComponentCounter;
			static ComponentID s_ComponentID{ e_ComponentCounter++ };

			return s_ComponentID;
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