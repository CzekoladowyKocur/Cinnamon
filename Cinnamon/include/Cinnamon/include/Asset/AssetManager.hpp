#pragma once
#include "Cinnamon/include/Asset/Asset.hpp"

namespace Cinnamon {
	class VulkanAllocator;
	class Texture2D;
	struct TextureSpecification;
	
	class AssetManager final
	{
	private:
		NON_COPYABLE(AssetManager)
	public:
		explicit AssetManager(const STL::Unique<VulkanAllocator>& deviceAllocator) noexcept;
		~AssetManager() noexcept;

		AssetHandle<Texture2D> LoadTexture(const STL::Filepath& path, const TextureSpecification& specification);

		void FreeLoadedAssets();
	private:
		const STL::Unique<VulkanAllocator>&					m_DeviceAllocator;
		STL::UMap<STL::Filepath, AssetHandle<Texture2D>>	m_LoadedTextures;
	};
}