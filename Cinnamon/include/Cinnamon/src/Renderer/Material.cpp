#include "Cinnamon/include/Renderer/Material.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"

namespace Cinnamon {
	Material::Material(const STL::Unique<Shader>& shader) noexcept
		:
		m_Shader(shader)
	{}

	Material::~Material() noexcept
	{}

	void Material::Invalidate()
	{
		STL::Vector<VkWriteDescriptorSet> writeDescriptorSets;
		for (const auto& textureWrite : m_TextureWriteslol)
		{
			writeDescriptorSets.emplace_back
			(
				VkWriteDescriptorSet
				{
					.sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
					.pNext{ nullptr },
					.dstSet{ m_Shader->GetDescriptorSetHandle(textureWrite.Set) },
					.dstBinding{ textureWrite.Binding },
					.dstArrayElement{ 0U },
					.descriptorCount{ 1U },
					.descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
					.pImageInfo{ &textureWrite.info },
					.pBufferInfo{ nullptr },
					.pTexelBufferView{ nullptr }
				}
			);
		}
		m_TextureWriteslol.clear();


		for (const auto& textureWrite : m_TextureWrites)
		{
			writeDescriptorSets.emplace_back
			(
				VkWriteDescriptorSet
				{
					.sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
					.pNext{ nullptr },
					.dstSet{ m_Shader->GetDescriptorSetHandle(textureWrite.Set) },
					.dstBinding{ textureWrite.Binding },
					.dstArrayElement{ 0U },
					.descriptorCount{ 1U },
					.descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
					.pImageInfo{ &textureWrite.Texture->GetDescriptorImageInfo() },
					.pBufferInfo{ nullptr },
					.pTexelBufferView{ nullptr }
				}
			);
		}
		
		m_TextureWrites.clear();
		if (not writeDescriptorSets.empty())
		{
			vkUpdateDescriptorSets(
				m_Shader->GetAllocator()->GetDevice()->GetLogicalDevice(),
				static_cast<uint32_t>(writeDescriptorSets.size()),
				writeDescriptorSets.data(),
				0U,
				nullptr);
		}
	}

	void Material::SetTexture(const STL::String& name, const VkDescriptorImageInfo& descriptor)
	{
		const ShaderResource& resource{ m_Shader->FindShaderResource(name) };
		m_TextureWriteslol.emplace_back
		(
			lol
			{
				.Set		{ resource.Set		},
				.Binding	{ resource.Binding	},
				.info		{ descriptor		}
			}
		);
	}

	void Material::SetTexture(const STL::String& name, Texture2D* const texture)
	{
		const ShaderResource& resource{ m_Shader->FindShaderResource(name) };
		m_TextureWrites.emplace_back
		(
			PendingDescriptorImageWrite
			{
				.Set		{ resource.Set		},
				.Binding	{ resource.Binding	},
				.Texture	{ texture			}
			}
		);
	}

	void Material::SetTextures(const STL::String& name, Texture2D* const textures, const size_t textureCount)
	{
		const ShaderResource& resource{ m_Shader->FindShaderResource(name) };
		m_TextureArrayWrites.emplace_back
		(
			PendingDescriptorImageArrayWrite
			{
				.Set		{ resource.Set						},
				.Binding	{ resource.Binding					},
				.Textures	{ textures, textures + textureCount }
			}
		);
	}
}