#include "Cinnamon/include/Asset/Asset.hpp"

namespace Cinnamon {
	Asset::Asset(
		const STL::Filepath& assetPath, 
		const EAssetType assetType,
		const EAssetFlags assetFlags) noexcept
		:
		m_AssetPath(assetPath),
		m_AssetType(assetType),
		m_AssetFlags(assetFlags)
	{}

	Asset::Asset(
		const EAssetType assetType, 
		const EAssetFlags assetFlags) noexcept
		:
		m_AssetPath(),
		m_AssetType(assetType),
		m_AssetFlags(assetFlags)
	{
		CIN_ASSERT(m_AssetFlags & EAssetFlags::CreatedFromData);
	}

	const STL::Filepath& Asset::GetAssetPath() const
	{
		CIN_ASSERT(not (m_AssetFlags & EAssetFlags::CreatedFromData))
		return m_AssetPath;
	}

	EAssetType Asset::GetAssetType() const
	{
		return m_AssetType;
	}

	EAssetFlags Asset::GetAssetFlags() const
	{
		return m_AssetFlags;
	}
}