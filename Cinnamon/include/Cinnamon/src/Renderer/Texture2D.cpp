#include "Cinnamon/include/Renderer/Texture2D.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"

#include "Cinnamon/include/Core/Filesystem.hpp"
#include "stb_image/stb_image.h"

namespace Cinnamon {
	InternalScope constexpr VkFilter CinnamonSamplerFilterToVulkanSamplerFilter(const ETextureSamplerFilterMode filterMode);
	InternalScope constexpr VkSamplerAddressMode CinnamonWrapModeToVulkanWrapMode(const ETextureSamplerWrapMode wrapMode);
	InternalScope constexpr VkSamplerMipmapMode CinnamonFilterModeToVulkanMipmapMode(const ETextureSamplerFilterMode filterMode);
	InternalScope uint32_t CalculateMipCount(const uint32_t width, const uint32_t height);

	Texture2D::Texture2D(
		const STL::Filepath& filepath,
		const STL::Unique<VulkanAllocator>& allocator,
		const TextureSpecification& specification) noexcept(true)
		:
		Asset(filepath, EAssetType::Texture),
		m_Allocator(allocator),
		m_Specification(specification),
		m_Width(0U),
		m_Height(0U),
		m_ChannelCount(0U),
		m_Format(EImageFormat::R8G8B8A8),
		m_TextureImage(VK_NULL_HANDLE),
		m_TextureImageView(VK_NULL_HANDLE),
		m_Sampler(VK_NULL_HANDLE),
		m_ImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		m_DeviceAllocation(),
		m_DescriptorImageInfo()
	{
		CIN_ASSERT(FileExists(filepath));
		const STL::String filepathString{ filepath.string() };

		int32_t width{ 0 }, height{ 0 }, channels{ 0 };
		stbi_uc* const pixelData{ stbi_load(filepathString.data(), &width, &height, &channels, STBI_rgb_alpha) };
		CIN_ASSERT(pixelData);

		if (pixelData)
		{
			m_Width = static_cast<uint32_t>(width);
			m_Height = static_cast<uint32_t>(height);
			m_ChannelCount = static_cast<uint32_t>(channels);

			const size_t size{ m_Width * m_Height * m_ChannelCount };
			const uint32_t mipCount{ CalculateMipCount(m_Width, m_Height) };
			
			CreateImage(pixelData, size, mipCount);
			CreateImageView();
			CreateSampler(mipCount);
			
			free(pixelData);
		}
	}

	Texture2D::Texture2D(
		const STL::Unique<VulkanAllocator>& allocator, 
		const void* const pixelData, 
		const uint32_t width, 
		const uint32_t height, 
		const EImageFormat format, 
		const TextureSpecification& specification)
		:
		Asset(EAssetType::Texture),
		m_Allocator(allocator),
		m_Specification(specification),
		m_Width(width),
		m_Height(height),
		m_ChannelCount(ChannelCountFromFormat(format)),
		m_Format(EImageFormat::R8G8B8A8),
		m_TextureImage(VK_NULL_HANDLE),
		m_TextureImageView(VK_NULL_HANDLE),
		m_Sampler(VK_NULL_HANDLE),
		m_ImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		m_DeviceAllocation(),
		m_DescriptorImageInfo()
	{
		const size_t size{ m_Width * m_Height * m_ChannelCount };
		const uint32_t mipCount{ CalculateMipCount(m_Width, m_Height) };

		CreateImage(pixelData, size, mipCount);
		CreateImageView();
		CreateSampler(mipCount);
	}

	Texture2D::~Texture2D() noexcept
	{
		[[likely]]
		if (m_Sampler)
		{
			vkDestroySampler(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_Sampler,
				GraphicsContext::GetAllocator());

			m_Sampler = VK_NULL_HANDLE;
		}
		
		[[likely]]
		if (m_TextureImageView)
		{
			vkDestroyImageView(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_TextureImageView,
				GraphicsContext::GetAllocator());

			m_TextureImageView = VK_NULL_HANDLE;
		}

		[[likely]]
		if(m_TextureImage and m_DeviceAllocation)
			m_Allocator->DestroyImage(m_TextureImage, m_DeviceAllocation);
	}

	void Texture2D::Invalidate(const TextureSpecification& specification)
	{
		CIN_UNIMPLEMENTED();
		m_Specification = specification;
	}

	const TextureSpecification& Texture2D::GetSpecification() const
	{
		return m_Specification;
	}
	
	uint32_t Texture2D::GetWidth() const
	{
		return m_Width;
	}

	uint32_t Texture2D::GetHeight() const
	{
		return m_Height;
	}

	std::pair<uint32_t, uint32_t> Texture2D::GetSize() const
	{
		return std::pair<uint32_t, uint32_t>{ m_Width, m_Height };
	}

	VkImage Texture2D::GetImage() const
	{
		return m_TextureImage;
	}

	VkImageView Texture2D::GetImageView() const
	{
		return m_TextureImageView;
	}

	VkSampler Texture2D::GetSampler() const
	{
		return m_Sampler;
	}

	VkImageLayout Texture2D::GetImageLayout() const
	{
		return m_ImageLayout;
	}

	VmaAllocation Texture2D::GetImageAllocation() const
	{
		return m_DeviceAllocation;
	}

	const VkDescriptorImageInfo& Texture2D::GetDescriptorImageInfo() const
	{
		CIN_ASSERT(
			m_DescriptorImageInfo.sampler	and 
			m_DescriptorImageInfo.imageView and 
			m_DescriptorImageInfo.imageLayout);

		return m_DescriptorImageInfo;
	}

	void Texture2D::CreateImage(const void* const pixelData, const size_t size, const uint32_t mipCount)
	{
		CIN_ASSERT(pixelData and size and mipCount);

		/* Create image */
		const VkImageCreateInfo imageCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.imageType{ VK_IMAGE_TYPE_2D },
			.format{ static_cast<VkFormat>(m_Format) },
			.extent
			{
				.width{ m_Width },
				.height{ m_Height },
				.depth{ 1 }
			},
			.mipLevels{ mipCount },
			.arrayLayers{ 1U },
			.samples{ VK_SAMPLE_COUNT_1_BIT },
			.tiling{ VK_IMAGE_TILING_OPTIMAL },
			.usage{ VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT },
			.sharingMode{ VK_SHARING_MODE_EXCLUSIVE },
			.queueFamilyIndexCount{ 0U },
			.pQueueFamilyIndices{ nullptr },
			.initialLayout{ VK_IMAGE_LAYOUT_UNDEFINED },
		};

		CIN_VERIFY(m_Allocator->AllocateImage(
			imageCreateInfo,
			VMA_MEMORY_USAGE_GPU_ONLY,
			m_DeviceAllocation,
			m_TextureImage));

		m_Allocator->GetDevice()->PerformSingleSubmitGraphicsOperation([this](const VkCommandBuffer imageMemoryBarrierCommandBuffer)
		{
			InsertImageMemoryBarrier
			(
				imageMemoryBarrierCommandBuffer,
				m_TextureImage,
				0, 0,
				VK_IMAGE_LAYOUT_UNDEFINED, m_ImageLayout,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VkImageSubresourceRange
				{
					.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
					.baseMipLevel{ 0U },
					.levelCount{ 1U },
					.baseArrayLayer{ 0U },
					.layerCount{ 1U }
				}
			);
		});

		const VkBufferCreateInfo stagingBufferCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.size{ size },
			.usage{ VK_BUFFER_USAGE_TRANSFER_SRC_BIT },
			.sharingMode{ VK_SHARING_MODE_EXCLUSIVE },
			.queueFamilyIndexCount{ 0U },
			.pQueueFamilyIndices{ nullptr }
		};

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferAllocation;

		CIN_VERIFY(m_Allocator->AllocateBuffer(
			stagingBufferCreateInfo,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			stagingBufferAllocation,
			stagingBuffer));

		void* const destination{ m_Allocator->MapMemory(stagingBufferAllocation) };
		memcpy(destination, pixelData, size);
		m_Allocator->UnmapMemory(stagingBufferAllocation);

		const VkImageMemoryBarrier imageMemoryBarrier
		{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER },
			.pNext{ nullptr },
			.srcAccessMask{ 0U },
			.dstAccessMask{ VK_ACCESS_TRANSFER_WRITE_BIT },
			.oldLayout{ VK_IMAGE_LAYOUT_UNDEFINED },
			.newLayout{ VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
			.srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
			.dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
			.image{ m_TextureImage },
			.subresourceRange
			{
				.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
				.baseMipLevel{ 0U },
				.levelCount{ mipCount },
				.baseArrayLayer{ 0U },
				.layerCount{ 1U }
			}
		};

		const VkBufferImageCopy imageBufferCopyRegion
		{
			.bufferOffset{ 0U },
			.bufferRowLength{ 0U },
			.bufferImageHeight{ 0U },
			.imageSubresource
			{
				.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
				.mipLevel{ 0U },
				.baseArrayLayer{ 0U },
				.layerCount{ 1U }
			},
			.imageOffset
			{
				.x{ 0 },
				.y{ 0 },
				.z{ 0 }
			},
			.imageExtent
			{
				.width{ m_Width },
				.height{ m_Height },
				.depth{ 1U }
			}
		};

		m_Allocator->GetDevice()->PerformSingleSubmitGraphicsOperation([&, this](const VkCommandBuffer imageDataCopyCommandBuffer)
		{
			/* Copy buffer contents to image */
			{
				vkCmdPipelineBarrier(
					imageDataCopyCommandBuffer,
					VK_PIPELINE_STAGE_HOST_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageMemoryBarrier);

				vkCmdCopyBufferToImage(
					imageDataCopyCommandBuffer,
					stagingBuffer,
					m_TextureImage,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&imageBufferCopyRegion);
			}

			/* Generate texture mip maps */
			{
				VkImageMemoryBarrier barrier
				{
					.sType{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER },
					.pNext{ nullptr },
					.srcAccessMask{ 0U },
					.dstAccessMask{ 0U },
					.oldLayout{ VK_IMAGE_LAYOUT_UNDEFINED },
					.newLayout{ VK_IMAGE_LAYOUT_UNDEFINED },
					.srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
					.dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
					.image{ m_TextureImage },
					.subresourceRange
					{
						.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
						.baseMipLevel{ 0U },
						.levelCount{ 1U },
						.baseArrayLayer{ 0U },
						.layerCount{ 1U }
					}
				};

				int32_t mipWidth{ static_cast<int32_t>(m_Width) }, mipHeight{ static_cast<int32_t>(m_Height) };
				for (uint32_t i{ 1U }; i < mipCount; ++i)
				{
					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.subresourceRange.baseMipLevel = i - 1U;

					/* Transfer mipped image */
					vkCmdPipelineBarrier(
						imageDataCopyCommandBuffer,
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
						0U,
						0U, nullptr,
						0U, nullptr,
						1U, &barrier);

					VkImageBlit imageBlit;
					imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.srcSubresource.mipLevel = i - 1U;
					imageBlit.srcSubresource.layerCount = 1U;
					imageBlit.srcSubresource.baseArrayLayer = 0U;

					imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.dstSubresource.mipLevel = i;
					imageBlit.dstSubresource.layerCount = 1U;
					imageBlit.dstSubresource.baseArrayLayer = 0U;

					imageBlit.srcOffsets[0U].x = 0;
					imageBlit.srcOffsets[0U].y = 0;
					imageBlit.srcOffsets[0U].z = 0;
					imageBlit.srcOffsets[1U].x = mipWidth;
					imageBlit.srcOffsets[1U].y = mipHeight;
					imageBlit.srcOffsets[1U].z = 1;

					imageBlit.dstOffsets[0U].x = 0;
					imageBlit.dstOffsets[0U].y = 0;
					imageBlit.dstOffsets[0U].z = 0;
					imageBlit.dstOffsets[1U].x = mipWidth > 1 ? mipWidth / 2 : 1;
					imageBlit.dstOffsets[1U].y = mipHeight > 1 ? mipHeight / 2 : 1;
					imageBlit.dstOffsets[1U].z = 1;

					vkCmdBlitImage(
						imageDataCopyCommandBuffer,
						m_TextureImage,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						m_TextureImage,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1U, &imageBlit,
						CinnamonSamplerFilterToVulkanSamplerFilter(m_Specification.SamplerFilterMode));

					barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
					barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					vkCmdPipelineBarrier(
						imageDataCopyCommandBuffer,
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						0U,
						0U, nullptr,
						0U, nullptr,
						1U, &barrier);

					mipWidth = mipWidth > 1 ? mipWidth / 2 : mipWidth;
					mipHeight = mipHeight > 1 ? mipHeight / 2 : mipHeight;
				}

				barrier.subresourceRange.baseMipLevel = mipCount - 1U;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(
					imageDataCopyCommandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0U,
					0U, nullptr,
					0U, nullptr,
					1U, &barrier);
			}
		});

		m_Allocator->DestroyBuffer(stagingBuffer, stagingBufferAllocation);
	}

	void Texture2D::CreateImageView()
	{
		/* Create image views */
		const VkImageViewCreateInfo imageViewCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.image{ m_TextureImage },
			.viewType{ VK_IMAGE_VIEW_TYPE_2D },
			.format{ static_cast<VkFormat>(m_Format) },
			.components
			{
				.r{ VK_COMPONENT_SWIZZLE_R },
				.g{ VK_COMPONENT_SWIZZLE_G },
				.b{ VK_COMPONENT_SWIZZLE_B },
				.a{ VK_COMPONENT_SWIZZLE_A }
			},
			.subresourceRange
			{
				.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
				.baseMipLevel{ 0U },
				.levelCount{ 1U },
				.baseArrayLayer{ 0U },
				.layerCount{ 1U }
			}
		};

		VK_CHECK(vkCreateImageView(
			m_Allocator->GetDevice()->GetLogicalDevice(),
			&imageViewCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_TextureImageView));
	}

	void Texture2D::CreateSampler(const uint32_t mipCount)
	{
		const VkSamplerCreateInfo samplerCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.magFilter{ CinnamonSamplerFilterToVulkanSamplerFilter(m_Specification.SamplerFilterMode) },
			.minFilter{ CinnamonSamplerFilterToVulkanSamplerFilter(m_Specification.SamplerFilterMode) },
			.mipmapMode{ CinnamonFilterModeToVulkanMipmapMode(m_Specification.SamplerFilterMode) },
			.addressModeU{ CinnamonWrapModeToVulkanWrapMode(m_Specification.SamplerWrapMode) },
			.addressModeV{ CinnamonWrapModeToVulkanWrapMode(m_Specification.SamplerWrapMode) },
			.addressModeW{ CinnamonWrapModeToVulkanWrapMode(m_Specification.SamplerWrapMode) },
			.mipLodBias{ 0.0f },
			.anisotropyEnable{ VK_FALSE },
			.maxAnisotropy{ 1.0f },
			.compareEnable{ VK_FALSE },
			.compareOp{ VK_COMPARE_OP_NEVER },
			.minLod{ 0.0f },
			.maxLod{ static_cast<float>(mipCount + 1U)},
			.borderColor{ VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE },
			.unnormalizedCoordinates{ VK_FALSE }
		};

		VK_CHECK(vkCreateSampler(
			m_Allocator->GetDevice()->GetLogicalDevice(),
			&samplerCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Sampler));

		m_DescriptorImageInfo.sampler = m_Sampler;
		m_DescriptorImageInfo.imageView = m_TextureImageView;
		m_DescriptorImageInfo.imageLayout = m_ImageLayout;
	}

	InternalScope constexpr VkFilter CinnamonSamplerFilterToVulkanSamplerFilter(const ETextureSamplerFilterMode filterMode)
	{
		switch (filterMode)
		{
			case ETextureSamplerFilterMode::Nearest:	return VK_FILTER_NEAREST;
			case ETextureSamplerFilterMode::Linear:		return VK_FILTER_LINEAR;

			default:
			{
				CIN_ASSERT(false, "Unknown filter mode");
				return static_cast<VkFilter>(0);
			}
		}
	}

	InternalScope constexpr VkSamplerAddressMode CinnamonWrapModeToVulkanWrapMode(const ETextureSamplerWrapMode wrapMode)
	{
		switch (wrapMode)
		{
			case ETextureSamplerWrapMode::Clamp:	return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case ETextureSamplerWrapMode::Repeat:	return VK_SAMPLER_ADDRESS_MODE_REPEAT;

			default:
			{
				CIN_ASSERT(false, "Unknown wrap mode");
				return static_cast<VkSamplerAddressMode>(0);
			}
		}
	}

	InternalScope constexpr VkSamplerMipmapMode CinnamonFilterModeToVulkanMipmapMode(const ETextureSamplerFilterMode filterMode)
	{
		switch (filterMode)
		{
			case ETextureSamplerFilterMode::Nearest:	return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			case ETextureSamplerFilterMode::Linear:		return VK_SAMPLER_MIPMAP_MODE_LINEAR;

			default:
			{
				CIN_ASSERT(false, "Unknown filter mode"); 
				return static_cast<VkSamplerMipmapMode>(0);
			}
		}
	}

	InternalScope uint32_t CalculateMipCount(const uint32_t width, const uint32_t height)
	{
		return static_cast<uint32_t>(std::floor(std::log2(std::min(width, height)))) + 1U;
	}
}