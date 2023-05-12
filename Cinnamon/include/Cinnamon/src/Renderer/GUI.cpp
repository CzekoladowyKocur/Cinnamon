#include "Cinnamon/include/GUI/GUI.hpp"
#include "Cinnamon/include/Core/Core.hpp"

#include "Cinnamon/include/Renderer/VulkanTypes.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/DescriptorPool.hpp"
/* For vulkan backend */
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_vulkan.h"

namespace Cinnamon {
	/* Used by vulkan backend to register textures */
	[[nodiscard]] InternalScope VkDescriptorSet ImGui_ImplVulkan_AddTextureUser(
		const STL::Unique<Renderer>& renderer,
		const VkSampler sampler,
		const VkImageView imageView,
		const VkImageLayout imageLayout);

	namespace GUI {
		void Image(
			const STL::Unique<Renderer>& renderer,
			const ImageViewID imageViewID,
			const float width,
			const float height)
		{
			/* Descriptor pool is set in renderer */
			const VkDescriptorSet descriptorSet{ ImGui_ImplVulkan_AddTextureUser(renderer, VK_NULL_HANDLE, VkImageView(imageViewID), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) };
			ImGui::Image(descriptorSet, ImVec2{ width, height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		}
	}
}

#ifdef CIN_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 4616)
#pragma warning(disable : 4100)
#pragma warning(disable : 4211)
#include "ThirdParty/imgui/backends/imgui_impl_vulkan.cpp"
#pragma warning(pop)
#else
#include "ThirdParty/imgui/backends/imgui_impl_vulkan.cpp"
#endif

namespace Cinnamon {
	InternalScope VkDescriptorSet ImGui_ImplVulkan_AddTextureUser(
		const STL::Unique<Renderer>& renderer,
		const VkSampler sampler,
		const VkImageView imageView,
		const VkImageLayout imageLayout)
	{
		ImGui_ImplVulkan_Data* const bd{ ImGui_ImplVulkan_GetBackendData() };

		const auto& descriptorPool{ renderer->GetDescriptorPool() };
		const uint32_t frameIndex{ renderer->GetFrameIndex() };

		VkDescriptorSet descriptorSet;
		{
			/* descriptor pool is set in descriptor pool */
			VkDescriptorSetAllocateInfo allocationInfo
			{
				.sType{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO },
				.pNext{ nullptr },
				.descriptorPool{ VK_NULL_HANDLE },
				.descriptorSetCount{ 1U },
				.pSetLayouts{ &bd->DescriptorSetLayout }
			};

			descriptorSet = descriptorPool->AllocateDescriptorSet(allocationInfo, frameIndex);
		}

		{
			ImGui_ImplVulkan_InitInfo* const v{ &bd->VulkanInitInfo };
			
			const VkDescriptorImageInfo descriptorImage 
			{
				.sampler{ sampler },
				.imageView{ imageView },
				.imageLayout{ imageLayout }
			};

			const VkWriteDescriptorSet writeDescriptor
			{
				.sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
				.pNext{ nullptr },
				.dstSet{ descriptorSet },
				.dstBinding{ 0U },
				.dstArrayElement{ 0U },
				.descriptorCount{ 1U },
				.descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
				.pImageInfo{ &descriptorImage },
				.pBufferInfo{ nullptr },
				.pTexelBufferView{ nullptr },
			};

			vkUpdateDescriptorSets(
				v->Device, 
				1U, 
				&writeDescriptor, 
				0U, 
				nullptr);
		}

		return descriptorSet;
	}
}