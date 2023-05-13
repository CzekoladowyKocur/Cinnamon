#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"

namespace Cinnamon {
	namespace ECS {
		class Registry;
	}

	class Scene final
	{
	private:
		NON_COPYABLE(Scene)
	public:
		explicit Scene() noexcept;
		~Scene() noexcept;

		[[nodiscard]] Entity
			CreateEntity(const STL::StringView entityName = "Unnamed Entity") noexcept;

		void DestroyEntity(Entity entity) noexcept;

		[[nodiscard]] STL::Unique<ECS::Registry>&
			GetRegistry() noexcept;

		[[nodiscard]] const STL::Unique<ECS::Registry>&
			GetRegistry() const noexcept;
	private:
		STL::Unique<ECS::Registry> m_Registry;
	private:
		friend class Entity;
	};
}