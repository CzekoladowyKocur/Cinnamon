#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	enum class EAssetType
	{
		Texture,
	};

	enum class EAssetFlags
	{
		Loaded,
		Unloaded,
	};

	template<typename AssetType>
	using AssetHandle = AssetType*;

	class Asset
	{
	private:
		NON_COPYABLE(Asset)
	public:
		explicit Asset(
			const STL::Filepath& assetPath,
			const EAssetType assetType) noexcept;

		virtual ~Asset() noexcept = default;

		[[nodiscard]] const STL::Filepath& 
			GetAssetPath() const;
			
		[[nodiscard]]EAssetType 
			GetAssetType() const;
	private:
		const STL::Filepath m_AssetPath;
		const EAssetType m_AssetType;
	};
}