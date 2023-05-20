#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Scene/SceneCamera.hpp"

namespace Cinnamon {
	namespace ECS {
		class Registry;
	}
	
	class PhysicsWorld2D;
	class Environment;

	enum class ESceneState
	{
		Paused,
		Playing,
		Edited,
	};

	class Scene final
	{
	public:
		explicit Scene(const ESceneState state) noexcept;
		~Scene() noexcept;

		void OnUpdate(const Timestep timestep);
		
		[[nodiscard]] Entity
			CreateEntity(const STL::StringView entityName = "Unnamed Entity") noexcept;

		void DestroyEntity(Entity entity) noexcept;
		void SetSceneState(const ESceneState sceneState);

		[[nodiscard]] ESceneState
			GetSceneState() const;
		
		[[nodiscard]] STL::Unique<ECS::Registry>&
			GetRegistry() noexcept;

		[[nodiscard]] const STL::Unique<ECS::Registry>&
			GetRegistry() const noexcept;

		[[nodiscard]] STL::Unique<Environment>&
			GetEnvironment() noexcept;

		[[nodiscard]] const STL::Unique<Environment>&
			GetEnvironment() const noexcept;

		[[nodiscard]] SceneCamera&
			GetPrimaryCamera() noexcept;

		[[nodiscard]] const SceneCamera&
			GetPrimaryCamera() const noexcept;	
	private:
		void OnScenePause(const ESceneState previousSceneState);
		void OnScenePlay(const ESceneState previousSceneState);
		void OnSceneEdit(const ESceneState previousSceneState); 

		void OnScenePausedUpdate(const Timestep timestep);
		void OnScenePlayedUpdate(const Timestep timestep);
		void OnSceneEditedUpdate(const Timestep timestep);
	private:
		STL::Unique<ECS::Registry>	m_Registry;
		STL::Unique<PhysicsWorld2D> m_PhysicsWorld;
		STL::Unique<Environment>	m_Environment;
		SceneCamera					m_DefaultCamera;
		ESceneState					m_SceneState;
	private:
		friend class Entity;
	};
}