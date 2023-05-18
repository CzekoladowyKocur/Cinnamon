#include "Cinnamon/include/Asset/Asset.hpp"

namespace Cinnamon {
	Asset::Asset(
		const STL::Filepath& assetPath, 
		const EAssetType assetType) noexcept
		:
		m_AssetPath(assetPath),
		m_AssetType(assetType)
	{}

	const STL::Filepath& Asset::GetAssetPath() const
	{
		return m_AssetPath;
	}

	EAssetType Asset::GetAssetType() const
	{
		return m_AssetType;
	}
}