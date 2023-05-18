#include "Cinnamon/include/Asset/AssetManager.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

namespace Cinnamon {
	AssetManager::AssetManager(const STL::Unique<VulkanAllocator>& deviceAllocator) noexcept
		:
		m_DeviceAllocator(deviceAllocator)
	{}

	AssetManager::~AssetManager() noexcept
	{
		FreeLoadedAssets();
	}

	AssetHandle<Texture2D> AssetManager::LoadTexture(const STL::Filepath& path, const TextureSpecification& specification)
	{
		if (m_LoadedTextures.contains(path))
		{
			AssetHandle<Texture2D> loadedTexture{ m_LoadedTextures[path] };
			/* Simply return the loaded texture */
			//if (specification == loadedTexture->GetSpecification())
				return loadedTexture;

			/* Else, modify the sampler */
			//loadedTexture->Invalidate(specification);
		}

		/* TODO: Error handling */
		AssetHandle<Texture2D> const texture{ cinew Texture2D(path, m_DeviceAllocator, specification) };
		m_LoadedTextures[path] = texture;

		return texture;
	}

	void AssetManager::FreeLoadedAssets()
	{
		for (const auto& [assetPath, asset] : m_LoadedTextures)
		{
			CIN_ASSERT(asset);
			CIN_INFO("Freeing texture {}", assetPath.string());
			
			cindel asset;
		}
	}
}