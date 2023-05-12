#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Renderer/Image.hpp"
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"

namespace Cinnamon {
	enum class ETextureSamplerWrapMode
	{
		None = -1,
		Clamp = 0,
		Repeat = 1,
		End,
	};

	enum class ETextureSamplerFilterMode
	{
		None = -1,
		Nearest = 0,
		Linear = 1,
		End,
	};

	struct TextureSpecification final
	{
		ETextureSamplerWrapMode SamplerWrapMode;
		ETextureSamplerFilterMode SamplerFilterMode;
	};

	class Texture2D final
	{
	private:
		NON_COPYABLE(Texture2D)
	public:
		explicit Texture2D(
			const STL::Unique<VulkanAllocator>& allocator,
			const STL::Filepath& filepath, 
			const TextureSpecification& specification) noexcept(true);
		
		~Texture2D() noexcept;

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
	};
}