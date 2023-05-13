#pragma once
#include "Cinnamon/include/ECS/ECS.hpp"

namespace Cinnamon {
	// ECS is bloated so it has it's own namespace.
	namespace ECS {			
		struct ComponentPool final
		{
			/* This needs to be fixed. Memory gets moved when the pool grows making all previous component references invalid. */
			static constexpr size_t s_InitialPoolSize{ 1000U };
		
			inline explicit ComponentPool(const size_t componentSize) noexcept(true)
				:
				m_ComponentSize(componentSize),
				m_ComponentBuffer({ .Capacity{ s_InitialPoolSize }, .Data{ reinterpret_cast<std::byte*>(malloc(s_InitialPoolSize * componentSize)) }})
			{}
		
			inline ~ComponentPool() noexcept(true)
			{
				[[likely]]
				if (m_ComponentBuffer.Data)
				{
					free(m_ComponentBuffer.Data);
					m_ComponentBuffer.Data = nullptr;
					m_ComponentBuffer.Capacity = 0U;
				}
			}
		
			template<typename ComponentType> requires ComponentConcept<ComponentType>
			ComponentType* Get(const EntityIndex entityIndex) noexcept(true)
			{
				return reinterpret_cast<ComponentType*>((m_ComponentBuffer.Data + (entityIndex * m_ComponentSize)));
			}
		
			template<typename ComponentType, typename... Args> requires ComponentConcept<ComponentType>
			ComponentType* Construct(const EntityIndex entityIndex, Args&&... args) noexcept(true)
			{
				const auto bufferIndex{ entityIndex + 1U };
				if (m_ComponentBuffer.Capacity >= bufferIndex)
				{
					return new (reinterpret_cast<void*>(m_ComponentBuffer.Data + (entityIndex * m_ComponentSize))) ComponentType(std::forward<Args>(args)...);
				}
				else
				{
					// Triple the current capacity
					const size_t newCapacity{ (m_ComponentBuffer.Capacity + 2) * 2U };
					std::byte* newBuffer{ (std::byte*)malloc(newCapacity * m_ComponentSize) };
					if (!newBuffer)
						std::abort();
		
					CIN_ASSERT(newBuffer);
					// Copy the current buffer
					memcpy(newBuffer, m_ComponentBuffer.Data, m_ComponentBuffer.Capacity * m_ComponentSize);
		
					[[likely]]
					if(m_ComponentBuffer.Data)
						free(m_ComponentBuffer.Data);
					
					m_ComponentBuffer.Data		= newBuffer;
					m_ComponentBuffer.Capacity	= newCapacity;
		
					return Construct<ComponentType>(entityIndex, std::forward<Args>(args)...);
				}
			}

			template<typename... Args>
			void* Allocate(const EntityIndex entityIndex) noexcept(true)
			{
				const auto bufferIndex{ entityIndex + 1U };
				if (m_ComponentBuffer.Capacity >= bufferIndex)
				{
					return reinterpret_cast<void*>(m_ComponentBuffer.Data + (entityIndex * m_ComponentSize));
				}
				else
				{
					// Triple the current capacity
					const size_t newCapacity{ (m_ComponentBuffer.Capacity + 2) * 2U };
					std::byte* newBuffer{ (std::byte*)malloc(newCapacity * m_ComponentSize) };
					if (!newBuffer)
						std::abort();

					CIN_ASSERT(newBuffer);
					// Copy the current buffer
					memcpy(newBuffer, m_ComponentBuffer.Data, m_ComponentBuffer.Capacity * m_ComponentSize);

					[[likely]]
					if (m_ComponentBuffer.Data)
						free(m_ComponentBuffer.Data);

					m_ComponentBuffer.Data = newBuffer;
					m_ComponentBuffer.Capacity = newCapacity;

					return Allocate(entityIndex);
				}
			}

			void* Get(const EntityIndex entityIndex) noexcept(true)
			{
				return reinterpret_cast<void*>((m_ComponentBuffer.Data + (entityIndex * m_ComponentSize)));
			}
		
			const size_t m_ComponentSize;
			struct
			{
				// Capacity of the buffer in terms of how many components it can fit.
				size_t		Capacity;
				std::byte*	Data;
			} m_ComponentBuffer;
		};

		class Registry final
		{
		private:
			NON_COPYABLE(Registry)
		public:
			struct Entity final
			{
				EntityID		ID;
				ComponentMask	Mask;
		
				constexpr operator std::pair<EntityID, ComponentMask>() const noexcept(true)
				{
					return std::pair<EntityID, ComponentMask>{ ID, Mask };
				}
			};
		public:
			explicit Registry() noexcept(true)
			{
				CIN_CRITICAL("Safe component pools for entities are needed (old memory can move if pool grows). Allocate much space for now.");
			}

			inline ~Registry() noexcept(true)
			{
				for (auto pool : m_ComponentPools)
					delete pool;
			}
	
			[[nodiscard]] inline EntityID CreateEntity() noexcept(true)
			{
				if (!m_FreeEntityIndices.empty())
				{
					// Fetch free index.
					const EntityIndex index{ m_FreeEntityIndices.back() };
					m_FreeEntityIndices.pop_back();
					// Create new entity in-place. Entity version gets updated each time the entity is destroyed.
					const EntityID entityID{ CreateEntityID(index, FetchEntityVersionFromID(m_Entities[index].ID)) };
					m_Entities[index] = { entityID, ComponentMask{} };
					return entityID;
				}
	
				return m_Entities.emplace_back(Entity{ CreateEntityID(static_cast<EntityIndex>(m_Entities.size()), 0U), ComponentMask{} }).ID;			
			}
	
			inline void DestroyEntity(const EntityID entityID) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));
	
				// Set the entity as invalid and update it's version.
				const EntityID invalidEntityID{ CreateEntityID(static_cast<EntityIndex>(Null), FetchEntityVersionFromID(entityID) + 1U) };
				const EntityIndex index{ FetchEntityIndexFromID(entityID) };
	
				m_Entities[index] = Entity{ invalidEntityID, ComponentMask{} };
				m_FreeEntityIndices.emplace_back(index);
			}
	
			template<typename ComponentType, typename... Args> requires ComponentConcept<ComponentType>
			ComponentType& Assign(const EntityID entityID, Args&&... args) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));
	
				const ComponentID componentID{ GetComponentID<ComponentType>() };
	
				/* Create a new pool for the specified component */
				if (m_ComponentPools.size() <= componentID)
					m_ComponentPools.resize(componentID + 1U, nullptr);
				
				if(!m_ComponentPools[componentID])
					m_ComponentPools[componentID] = new ComponentPool(sizeof(ComponentType));
	
				const EntityIndex entityIndex{ FetchEntityIndexFromID(entityID) };
				ComponentType* component{ m_ComponentPools[componentID]->Construct<ComponentType>(entityIndex, std::forward<Args>(args)...) };
	
				m_Entities[entityIndex].Mask.set(componentID);
				return *component;
			}
	
			template<typename ComponentType> requires ComponentConcept<ComponentType>
			void Remove(const EntityID entityID) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));
	
				const ComponentID componentID{ GetComponentID<ComponentType>() };
				m_Entities[FetchEntityIndexFromID(entityID)].Mask.reset(componentID);
			}
	
			template<typename ComponentType> requires ComponentConcept<ComponentType>
			bool Has(const EntityID entityID) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));
	
				const ComponentID componentID{ GetComponentID<ComponentType>() };
	
				return m_Entities[FetchEntityIndexFromID(entityID)].Mask.test(componentID);
			}
	
			template<typename ComponentType> requires ComponentConcept<ComponentType>
			ComponentType& Get(const EntityID entityID) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));
	
				const ComponentID componentID{ GetComponentID<ComponentType>() };
				ComponentType* const component{ reinterpret_cast<ComponentType*>(m_ComponentPools[componentID]->Get<ComponentType>(FetchEntityIndexFromID(entityID))) };
				return *component;
			}

			void* Assign(const EntityID entityID, const ComponentID componentID, const size_t componentSize) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));

				/* Create a new pool for the specified component */
				if (m_ComponentPools.size() <= componentID)
					m_ComponentPools.resize(componentID + 1U, nullptr);

				if (!m_ComponentPools[componentID])
					m_ComponentPools[componentID] = new ComponentPool(componentSize);

				const EntityIndex entityIndex{ FetchEntityIndexFromID(entityID) };
				void* componentMemory{ m_ComponentPools[componentID]->Allocate(entityIndex) };

				m_Entities[entityIndex].Mask.set(componentID);
				return componentMemory;
			}

			void* Get(const EntityID entityID, const ComponentID componentID) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));

				void* const component{ m_ComponentPools[componentID]->Get(FetchEntityIndexFromID(entityID)) };
				return component;
			}

			bool Has(const EntityID entityID, const ComponentID componentID) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));

				return m_Entities[FetchEntityIndexFromID(entityID)].Mask.test(componentID);
			}

			void Remove(const EntityID entityID, const ComponentID componentID) noexcept(true)
			{
				CIN_ASSERT(IsEntityValid(entityID));
				CIN_ASSERT(std::find_if(std::begin(m_Entities), std::end(m_Entities), [entityID](const auto& entity)
				{
					return entityID == entity.ID;
				}) != std::end(m_Entities));

				m_Entities[FetchEntityIndexFromID(entityID)].Mask.reset(componentID);
			}
	
			inline void Reset() noexcept(true)
			{
				for (auto pool : m_ComponentPools)
					delete pool;
			
				m_ComponentPools.clear();
				m_FreeEntityIndices.clear();
				m_Entities.clear();
			}
		public:
			/* Entity ID -> Entity Component Mask */
			STL::Vector<Entity> m_Entities;
			/* List of free entities */
			STL::Vector<EntityIndex> m_FreeEntityIndices;
			/* Component Pools */
			STL::Vector<ComponentPool*> m_ComponentPools;
		};

		template<typename... ComponentTypes>
		struct View final
		{
			// Viewed registry
			const Registry*						ViewedRegistry;
			// Viewed components
			const ComponentMask					Mask;
			// Set to true if no components were specified
			const bool							ViewAll;
		
			constexpr explicit View(const STL::Unique<Registry>& registry) noexcept(true)
				:
				ViewedRegistry(registry.get()),
				Mask
				(
					[]() -> const ComponentMask
					{
						if constexpr (sizeof...(ComponentTypes) == 0U)
							return ComponentMask{};
						else 
						{
							ComponentMask componentMask;
							
							const ComponentID componentIDS[]{ 0U, GetComponentID<ComponentTypes>() ... };
							// Skip first "component" id
							for (size_t i{ 1U }; i < (sizeof...(ComponentTypes) + 1U); i++)
								componentMask.set(componentIDS[i]);
				
							return componentMask;
						}
					}()
				),
				ViewAll(sizeof...(ComponentTypes) == 0U)
			{}
		
			struct Iterator
			{
				const Registry*		ViewedRegistry;
				const ComponentMask	Mask;
				const bool			ViewAll;
				EntityIndex			Index;
		
				constexpr explicit Iterator
				(
					const Registry*		registry,
					const ComponentMask	mask,
					const bool			viewAll,
					const EntityIndex	index
				) noexcept(true)
					:
					ViewedRegistry(registry),
					Mask(mask),
					ViewAll(viewAll),
					Index(index)
				{}
		
				inline EntityID operator*() const noexcept(true)
				{
					CIN_ASSERT(IsEntityValid(ViewedRegistry->m_Entities[Index].ID));
					return ViewedRegistry->m_Entities[Index].ID;
				}
		
				inline bool operator==(const Iterator& other) const noexcept(true)
				{
					return Index == other.Index || Index == ViewedRegistry->m_Entities.size();
				}
		
				inline bool operator!=(const Iterator& other) const noexcept(true)
				{
					return !(*this == other);
				}
		
				inline bool IsValidIndex() const noexcept(true)
				{
					const EntityID entityID{ ViewedRegistry->m_Entities[Index].ID };
					const ComponentMask entityMask{ ViewedRegistry->m_Entities[Index].Mask };
		
					return IsEntityValid(entityID) && (ViewAll || Mask == (Mask & entityMask));
				}
		
				inline Iterator& operator++() noexcept(true)
				{
					do
					{
						++Index;
					} while (Index < ViewedRegistry->m_Entities.size() && !IsValidIndex());
					return *this;
				}
			};
		
			inline const Iterator begin() const noexcept(true)
			{
				EntityIndex begin{ 0U };
		
				while
				(
					// Check if any entities are left
					(begin < ViewedRegistry->m_Entities.size()) &&
					(
						// Check if entity mask matches
						Mask != (Mask & ViewedRegistry->m_Entities[begin].Mask) ||
						// Check if entity is valid
						!IsEntityValid(ViewedRegistry->m_Entities[begin].ID)
					)
				)
				{
					++begin;
				}
		
				return Iterator(ViewedRegistry, Mask, ViewAll, begin);
			}
		
			inline const Iterator end() const noexcept(true)
			{
				return Iterator(ViewedRegistry, Mask, ViewAll, static_cast<EntityIndex>(ViewedRegistry->m_Entities.size()));
			}
		};
	}
}