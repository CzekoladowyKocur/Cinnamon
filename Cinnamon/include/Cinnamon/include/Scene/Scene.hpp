#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Scene/SceneCamera.hpp"

namespace Cinnamon {
	namespace ECS {
		class Registry;
	}
	
	class Environment;

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

		[[nodiscard]] STL::Unique<Environment>&
			GetEnvironment() noexcept;

		[[nodiscard]] const STL::Unique<Environment>&
			GetEnvironment() const noexcept;

		[[nodiscard]] const SceneCamera&
			GetPrimaryCamera() const noexcept;

		[[nodiscard]] SceneCamera&
			GetPrimaryCamera() noexcept;
	private:
		STL::Unique<ECS::Registry> m_Registry;
		STL::Unique<Environment> m_Environment;
		SceneCamera m_DefaultCamera;
	private:
		friend class Entity;
	};
}