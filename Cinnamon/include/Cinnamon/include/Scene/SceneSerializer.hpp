#pragma once
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"

namespace YAML {
	class Emitter;
}

namespace Cinnamon {
	class AssetManager;

	class SceneSerializer final
	{
	private:
		NON_COPYABLE(SceneSerializer)
	public:
		explicit SceneSerializer(Scene* const scene, const STL::Unique<AssetManager>& assetManager) noexcept;
		~SceneSerializer() noexcept = default;

		Errr operator<<(const STL::Filepath& filepath);
		Errr operator>>(const STL::Filepath& filepath);
	private:
		Errr Serialize(const STL::Filepath& filepath) noexcept(false);
		Errr Deserialize(const STL::Filepath& filepath) noexcept(false);
		
		void SerializeEntity(YAML::Emitter& emitter, const Entity entity);
	private:
		Scene* const m_Scene;
		const STL::Unique<AssetManager>& m_AssetManager;
	};
}