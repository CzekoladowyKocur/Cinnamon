#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Asset/Asset.hpp"
#include "Cinnamon/include/Renderer/Image.hpp"
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"

namespace Cinnamon {
	enum class ETextureSamplerWrapMode
	{
		Clamp = 0U,
		Repeat
	};

	enum class ETextureSamplerFilterMode
	{
		Nearest = 0U,
		Linear
	};

	struct TextureSpecification final
	{
		ETextureSamplerWrapMode SamplerWrapMode;
		ETextureSamplerFilterMode SamplerFilterMode;

		constexpr bool operator==(const TextureSpecification& other) const noexcept
		{
			return SamplerWrapMode == other.SamplerWrapMode and SamplerFilterMode == other.SamplerFilterMode;
		}
	};

	class Texture2D final : public Asset
	{
	private:
		NON_COPYABLE(Texture2D)
	public:
		explicit Texture2D(
			const STL::Filepath& filepath, 
			const STL::Unique<VulkanAllocator>& allocator,
			const TextureSpecification& specification) noexcept(true);
		
		~Texture2D() noexcept;

		void Invalidate(const TextureSpecification& specification);

		[[nodiscard]] const TextureSpecification&
			GetSpecification() const;

		[[nodiscard]] uint32_t
			GetWidth() const;

		[[nodiscard]] uint32_t
			GetHeight() const;

		[[nodiscard]] std::pair<uint32_t, uint32_t>
			GetSize() const;

		[[nodiscard]] VkImage
			GetImage() const;

		[[nodiscard]] VkImageView
			GetImageView() const;

		[[nodiscard]] VkSampler
			GetSampler() const;

		[[nodiscard]] VkImageLayout
			GetImageLayout() const;

		[[nodiscard]] VmaAllocation
			GetImageAllocation() const;

		[[nodiscard]] const VkDescriptorImageInfo& 
			GetDescriptorImageInfo() const;
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;

		TextureSpecification m_Specification;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_ChannelCount;
		EImageFormat m_Format;

		VkImage m_TextureImage;
		VkImageView m_TextureImageView;
		VkSampler m_Sampler;
		VkImageLayout m_ImageLayout;
		VmaAllocation m_DeviceAllocation;

		VkDescriptorImageInfo m_DescriptorImageInfo;
	};
}