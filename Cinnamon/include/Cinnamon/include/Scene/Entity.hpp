#pragma once
#include "Cinnamon/include/ECS/ECS.hpp"

namespace Cinnamon {
	class Scene;

	class Entity final
	{
	public:
		constexpr explicit Entity() noexcept
			:
			m_Scene(nullptr),
			m_EntityID(ECS::Null)
		{
			CIN_ASSERT(not ECS::IsEntityValid(m_EntityID));
		}

		constexpr explicit Entity(const ECS::EntityID ID, Scene* const scene)
			:
			m_Scene(scene),
			m_EntityID(ID)
		{}

		template<typename T, typename ... Args> requires ECS::ComponentConcept<T>
		inline T& AddComponent(Args&& ... args) noexcept
		{
			T* const componentMemory{ reinterpret_cast<T*>(AddComponentInternal(ECS::GetComponentID<T>(), sizeof(T))) };
			CIN_ASSERT(componentMemory);
				
			new (componentMemory) T(std::forward<Args>(args)...);
			return *componentMemory;
		}

		template<typename T> requires ECS::ComponentConcept<T>
		[[nodiscard]] inline T& GetComponent() noexcept
		{
			T* const componentMemory{ reinterpret_cast<T*>(GetComponentInternal(ECS::GetComponentID<T>())) };
			CIN_ASSERT(componentMemory);

			return *componentMemory;
		}

		template<typename T> requires ECS::ComponentConcept<T>
		[[nodiscard]] inline const T& GetComponent() const noexcept
		{
			T* const componentMemory{ reinterpret_cast<T*>(GetComponentInternal(ECS::GetComponentID<T>())) };
			CIN_ASSERT(componentMemory);

			return *componentMemory;
		}

		template<typename T> requires ECS::ComponentConcept<T>
		[[nodiscard]] inline bool HasComponent() const noexcept
		{
			return HasComponentInternal(ECS::GetComponentID<T>());
		}

		template<typename T> requires ECS::ComponentConcept<T>
		inline void RemoveComponent() noexcept
		{
			CIN_ASSERT(HasComponentInternal(ECS::GetComponentID<T>()));
			T* const componentMemory{ reinterpret_cast<T*>(GetComponentInternal(ECS::GetComponentID<T>())) };
			CIN_ASSERT(componentMemory);
			componentMemory->~T();

			RemoveComponentInternal(ECS::GetComponentID<T>());
		}

		constexpr bool operator==(const Entity other) const noexcept
		{
			/* Probably valid? */
			return m_EntityID == other.m_EntityID and m_Scene == other.m_Scene;
		}

		constexpr bool operator!=(const Entity other) const noexcept
		{
			return not (*this == other);
		}

		constexpr operator ECS::EntityID() const noexcept
		{
			return m_EntityID;
		}

		constexpr operator bool() const noexcept 
		{
			return m_Scene and m_EntityID != ECS::Null;
		}
	private:
		[[nodiscard]] void* AddComponentInternal(const ECS::ComponentID componentID, const size_t componentSize) noexcept;
		[[nodiscard]] void* GetComponentInternal(const ECS::ComponentID componentID) noexcept;
		[[nodiscard]] void* GetComponentInternal(const ECS::ComponentID componentID) const noexcept;
		[[nodiscard]] bool HasComponentInternal(const ECS::ComponentID componentID) const noexcept;
		void RemoveComponentInternal(const ECS::ComponentID componentID) noexcept;
	private:
		Scene* m_Scene;
		ECS::EntityID m_EntityID;
	};
}