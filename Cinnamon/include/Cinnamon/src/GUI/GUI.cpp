#include "Cinnamon/include/GUI/GUI.hpp"
#include "Cinnamon/include/GUI/Icons.hpp"
#include "Cinnamon/include/GUI/GUIRenderer.hpp"
#include "Cinnamon/include/Core/Core.hpp"

#include "Cinnamon/include/Renderer/VulkanTypes.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/DescriptorPool.hpp"
/* For vulkan backend */
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"
#include "ThirdParty/imgui/backends/imgui_impl_vulkan.h"

namespace Cinnamon {
	/* Used by vulkan backend to register textures */
	[[nodiscard]] InternalScope VkDescriptorSet ImGui_ImplVulkan_AddTextureUser(
		const STL::Unique<Renderer>& renderer,
		const VkSampler sampler,
		const VkImageView imageView,
		const VkImageLayout imageLayout);

	extern GUIRenderer* s_GUIRenderer;

	namespace GUI {
		void Image(
			const ImageViewID imageViewID,
			const float width,
			const float height,
			const ImageSamplerID sampler,
			const bool flip)
		{
			/* TODO: temporary */
			CIN_ASSERT(s_GUIRenderer);
			/* Descriptor pool is set in renderer */
			const VkDescriptorSet descriptorSet{ ImGui_ImplVulkan_AddTextureUser(s_GUIRenderer->GetRenderer(), VkSampler(sampler), VkImageView(imageViewID), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) };
			if (flip)
				ImGui::Image(descriptorSet, ImVec2{ width, height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
			else
				ImGui::Image(descriptorSet, ImVec2{ width, height }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });
		}

		bool SearchBar(
			STL::String& outSearch,
			const bool spanAvailableWidth)
		{
			constexpr uint32_t MaxSearchBarInputBufferSize{ 128U };

			std::array<char, MaxSearchBarInputBufferSize> inputBuffer{ '\0' };
			const std::size_t size = CIN_CLAMP(outSearch.size(), outSearch.size(), MaxSearchBarInputBufferSize);
			std::copy_n(outSearch.begin(), size, inputBuffer.data());

			if (spanAvailableWidth)
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

			bool updated{ false };
			if (ImGui::InputTextWithHint("Search bar", "Search " ICON_FA_SEARCH, inputBuffer.data(), inputBuffer.size()))
			{
				outSearch = std::move(std::string(inputBuffer.data()));
				updated = true;
			}

			if (spanAvailableWidth)
				ImGui::PopItemWidth();

			return updated;
		}

		void Vec1Slider(
			const STL::StringView label,
			float values[1U],
			float resetValue,
			float width)
		{
			const float lineHeight{ GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f };
			const float buttonWidth{ ImGui::CalcTextSize("X").x + GImGui->Style.FramePadding.x };
			const ImVec2 buttonSize{ buttonWidth, lineHeight };
			const float dragfloatWidth{ width * 0.333f - buttonSize.x };

			ImGui::PushID(label.data());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.65f, 0.0f, 0.05f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.8f, 0.1f, 0.10f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.0f, 0.05f, 1.0f });

			if (ImGui::Button("X", buttonSize))
				values[0U] = resetValue;

			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushItemWidth(dragfloatWidth);
			ImGui::DragFloat("##X", &values[0U], 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();
			ImGui::PopID();
		}

		void Vec3Slider(
			const STL::StringView label, 
			float values[3U], 
			float resetValue, 
			float width)
		{
			const float lineHeight{ GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f };
			const float buttonWidth{ ImGui::CalcTextSize("X").x  + GImGui->Style.FramePadding.x };
			const ImVec2 buttonSize{ buttonWidth, lineHeight };
			const float dragfloatWidth{ width * 0.333f - buttonSize.x };

			ImGui::PushID(label.data());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.65f, 0.0f, 0.05f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.8f, 0.1f, 0.10f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.65f, 0.0f, 0.05f, 1.0f });

			if (ImGui::Button("X", buttonSize))
				values[0U] = resetValue;

			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushItemWidth(dragfloatWidth);
			ImGui::DragFloat("##X", &values[0U], 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.05f, 0.65f, 0.05f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.8f, 0.1f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.05f, 0.65f, 0.05f, 1.0f });
			if (ImGui::Button("Y", buttonSize))
				values[1U] = resetValue;

			ImGui::PopStyleColor(3);
			ImGui::SameLine();
			ImGui::PushItemWidth(dragfloatWidth);
			ImGui::DragFloat("##Y", &values[1U], 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.05f, 0.05f, 0.65f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.1f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.05f, 0.05f, 0.65f, 1.0f });
			if (ImGui::Button("Z", buttonSize))
				values[2U] = resetValue;

			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			ImGui::PushItemWidth(dragfloatWidth);
			ImGui::DragFloat("##Z", &values[2U], 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();
			ImGui::PopID();
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