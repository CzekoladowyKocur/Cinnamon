#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	enum class EAssetType
	{
		Texture,
	};

	enum class EAssetFlags
	{
		None			= 0U,
		Loaded			= BIT(1U),
		Unloaded		= BIT(2U),
		CreatedFromData = BIT(3U)
	};

	constexpr bool operator&(const EAssetFlags lhs, const EAssetFlags rhs) noexcept
	{
		return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
	}

	constexpr EAssetFlags operator|(const EAssetFlags lhs, const EAssetFlags rhs) noexcept
	{
		return static_cast<EAssetFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	template<typename AssetType>
	using AssetHandle = AssetType*;

	class Asset
	{
	private:
		NON_COPYABLE(Asset)
	public:
		explicit Asset(
			const STL::Filepath& assetPath,
			const EAssetType assetType,
			const EAssetFlags assetFlags = EAssetFlags::None) noexcept;

		explicit Asset(
			const EAssetType assetType,
			const EAssetFlags assetFlags = EAssetFlags::Loaded | EAssetFlags::CreatedFromData) noexcept;

		virtual ~Asset() noexcept = default;

		[[nodiscard]] const STL::Filepath& 
			GetAssetPath() const;
			
		[[nodiscard]] EAssetType 
			GetAssetType() const;

		[[nodiscard]] EAssetFlags
			GetAssetFlags() const;
	private:
		const STL::Filepath m_AssetPath;
		const EAssetType m_AssetType;
		mutable EAssetFlags m_AssetFlags;
	};
}