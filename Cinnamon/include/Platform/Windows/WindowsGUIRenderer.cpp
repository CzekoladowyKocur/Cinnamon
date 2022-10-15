#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/GUI/GUIRenderer.h"
#include "Cinnamon/include/GUI/Icons.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"
#include "Cinnamon/include/Renderer/Swapchain.h"
#include "Cinnamon/include/Core/Window.h"

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 4616)
#pragma warning(disable : 4100)
#pragma warning(disable : 4211)
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_vulkan.h"
#include "ThirdParty/imgui/backends/imgui_impl_win32.h"

namespace Cinnamon {
	InternalScope VkDescriptorPool s_DescriptorPool{ VK_NULL_HANDLE };
	InternalScope VkCommandPool s_CommandPool{ VK_NULL_HANDLE };
	InternalScope STL::Vector<VkCommandBuffer> s_CommandBuffers;
	InternalScope EUITheme s_UITheme{ EUITheme::Default };

	bool GUIRenderer::Initialize(const Window* const window)
	{
		IMGUI_CHECKVERSION();
		if (!ImGui::CreateContext())
			return false;

		ImGuiIO& io{ ImGui::GetIO() };
		ImGuiStyle& style{ ImGui::GetStyle() };
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		/* TODO: Revisit (causes input and renderpass errors) */
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		CIN_UNUSED(style);

		SetTheme(s_UITheme);
		constexpr std::size_t descriptorPoolCommonResourceSize = 100U;
		constexpr std::size_t descriptorPoolUncommonResourceSize = 10U;

		constexpr VkDescriptorPoolSize descriptorPoolSizes[11U]
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, descriptorPoolCommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorPoolCommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptorPoolCommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorPoolCommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptorPoolUncommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptorPoolUncommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorPoolUncommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorPoolUncommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptorPoolUncommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptorPoolUncommonResourceSize },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptorPoolUncommonResourceSize }
		};

		const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT },
			.maxSets{ ((4U * descriptorPoolCommonResourceSize + 7U * descriptorPoolUncommonResourceSize) / 11U) * CIN_CARRAY_SIZE(descriptorPoolSizes) },
			.poolSizeCount{ CIN_CARRAY_SIZE(descriptorPoolSizes) },
			.pPoolSizes{ descriptorPoolSizes },
		};

		VK_CHECK(vkCreateDescriptorPool(
			GraphicsContext::GetDevice(),
			&descriptorPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&s_DescriptorPool));

		ImGui_ImplVulkan_InitInfo vulkanInitializeInfo{
			.Instance{ GraphicsContext::GetInstance() },
			.PhysicalDevice{ GraphicsContext::GetPhysicalDevice() },
			.Device{ GraphicsContext::GetDevice() },
			.QueueFamily{ GraphicsContext::GetQueueFamily(GraphicsContext::EQueueFamily::Graphics) },
			.Queue{ GraphicsContext::GetGraphicsQueue() },
			.PipelineCache{ VK_NULL_HANDLE },
			.DescriptorPool{ s_DescriptorPool },
			.Subpass{ 0U },
			.MinImageCount{ 3 },
			.ImageCount{ GraphicsContext::GetSwapchainImageCount() },
			.MSAASamples{ VK_SAMPLE_COUNT_1_BIT },
			.Allocator{ GraphicsContext::GetAllocator() },
#ifdef CIN_DEBUG
			.CheckVkResultFn{ [](const VkResult result)
			{
				if (result != VK_SUCCESS)
				{
					if (result == VK_ERROR_DEVICE_LOST)
					{
						CIN_ERROR("{}", result);
						CIN_ASSERT(false);
					}

					if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
					{
						CIN_ERROR("{}", result);
						CIN_ASSERT(false);
					}

					CIN_ERROR("{}", result);
					CIN_ASSERT(false);
				}
			} },
#else
			.CheckVkResultFn{ nullptr }
#endif
		};

		CIN_VERIFY(ImGui_ImplWin32_Init(
			const_cast<void*>(const_cast<Window*>(window)->GetNativeHandle())));

		CIN_VERIFY(ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* userData)
			{
				return vkGetInstanceProcAddr(
					GraphicsContext::GetInstance(),
					function_name);	
			}));

		ImGui::GetPlatformIO().Platform_CreateVkSurface = [](ImGuiViewport* viewport, ImU64 vulkanInstance, const void* allocator, ImU64* outVulkanSurface) {
			const VkWin32SurfaceCreateInfoKHR createInfo{
				.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR} ,
				.pNext{ nullptr },
				.flags{ 0U },
				.hinstance{  GetModuleHandle(NULL) },
				.hwnd{ reinterpret_cast<HWND>(viewport->PlatformHandleRaw) },
			};
		
			VK_CHECK(vkCreateWin32SurfaceKHR(
				reinterpret_cast<VkInstance>(vulkanInstance),
				&createInfo,
				GraphicsContext::GetAllocator(),
				reinterpret_cast<VkSurfaceKHR*>(outVulkanSurface)));
		
			return static_cast<int>(VK_SUCCESS);
		};

		CIN_VERIFY(ImGui_ImplVulkan_Init(
			&vulkanInitializeInfo,
			GraphicsContext::GetSwapchainRenderPass()));

		UploadFontAtlas();
		UploadIconFontAtlas();
		
		GraphicsContext::PerformSingleSubmitGraphicsOperation([](VkCommandBuffer fontCommandBuffer)
			{
				ImGui_ImplVulkan_CreateFontsTexture(fontCommandBuffer);
			});

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		const VkCommandPoolCreateInfo commandPoolCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | 
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT /* Command buffers will be short lived */
		},
			.queueFamilyIndex{ GraphicsContext::GetQueueFamily(GraphicsContext::EQueueFamily::Graphics) },
		};

		VK_CHECK(vkCreateCommandPool(
			GraphicsContext::GetDevice(),
			&commandPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&s_CommandPool));

		const VkCommandBufferAllocateInfo imGuiCommandBufferAllocateInfo{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
			.pNext{ nullptr },
			.commandPool{ s_CommandPool },
			.level{ VK_COMMAND_BUFFER_LEVEL_SECONDARY },
			.commandBufferCount{ GraphicsContext::GetSwapchainImageCount() },
		};

		s_CommandBuffers.resize(GraphicsContext::GetSwapchainImageCount());
		VK_CHECK(vkAllocateCommandBuffers(
			GraphicsContext::GetDevice(),
			&imGuiCommandBufferAllocateInfo,
			&s_CommandBuffers[0]));

		return true;
	}

	void GUIRenderer::SetTheme(const EUITheme theme)
	{
		auto& style{ ImGui::GetStyle() };
		auto& colors{ ImGui::GetStyle().Colors };
		switch (theme)
		{
			case EUITheme::Dark:
			{
				colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
				colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
				colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
				colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
				colors[ImGuiCol_PopupBg] = ImVec4(0.200f, 0.200f, 0.200f, 0.80f);
				colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 1.000f);
				colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0);
				colors[ImGuiCol_FrameBg] = ImVec4(0.106f, 0.106f, 0.106f, 1.0f);
				colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
				colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
				colors[ImGuiCol_TitleBg] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
				colors[ImGuiCol_TitleBgActive] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
				colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
				colors[ImGuiCol_MenuBarBg] = ImVec4(0.121f, 0.121f, 0.121f, 1.000f);
				colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.0f);
				colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
				colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
				colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
				colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
				colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
				colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
				colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
				colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
				colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
				colors[ImGuiCol_Header] = ImVec4(0.218f, 0.218f, 0.218f, 1.000f);
				colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
				colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
				colors[ImGuiCol_Border] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
				colors[ImGuiCol_Separator] = ImVec4(0.135f, 0.135f, 0.135f, 1.0f);
				colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
				colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
				colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
				colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
				colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 1.0f, 1.000f, 1.000f);
				colors[ImGuiCol_Tab] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
				colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
				colors[ImGuiCol_TabActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
				colors[ImGuiCol_TabUnfocused] = ImVec4(0.098f, 0.098f, 0.098f, 1.000f);
				colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
				colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
				colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
				colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
				colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 1.000f, 1.000f);
				colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
				colors[ImGuiCol_DragDropTarget] = { 204.0f / 255.0f, 164.0f / 255.0f, 61.0f / 255.0f, 1.0f };
				colors[ImGuiCol_DockingPreview] = { 204.0f / 255.0f, 164.0f / 255.0f, 61.0f / 255.0f, 1.0f };
				colors[ImGuiCol_DockingEmptyBg] = { 0.15f, 0.15f, 0.15f, 1.0f };
				colors[ImGuiCol_DockingPreview] = { 204.0f / 255.0f, 164.0f / 255.0f, 61.0f / 255.0f, 1.0f };
				colors[ImGuiCol_NavHighlight] = { 204.0f / 255.0f, 164.0f / 255.0f, 61.0f / 255.0f, 1.0f };
				colors[ImGuiCol_NavWindowingHighlight] = { 204.0f / 255.0f, 164.0f / 255.0f, 61.0f / 255.0f, 1.0f };
				colors[ImGuiCol_NavWindowingDimBg] = { 204.0f / 255.0f, 164.0f / 255.0f, 61.0f / 255.0f, 1.0f };
				colors[ImGuiCol_ModalWindowDimBg] = { 204.0f / 255.0f, 164.0f / 255.0f, 61.0f / 255.0f, 0.03f };

				style.ChildRounding = 4.0f;
				style.FrameBorderSize = 0.0f;
				style.FrameRounding = 2.0f;
				style.GrabMinSize = 7.0f;
				style.PopupRounding = 2.0f;
				style.ScrollbarRounding = 0;
				style.ScrollbarSize = 12.0f;
				style.TabBorderSize = 0.0f;
				style.TabRounding = 0.0f;
				style.WindowRounding = 0.0f;
				style.IndentSpacing = 11.0f;
				style.PopupBorderSize = 1.0f;
				style.AntiAliasedLines = true;
				style.AntiAliasedFill = true;
				style.AntiAliasedLinesUseTex = true;
			} break;

			default:
			{
				CIN_ASSERT(false, "Invalid theme");
			} break;
		}
	}

	bool GUIRenderer::Shutdown()
	{
		VK_CHECK(vkDeviceWaitIdle(
			GraphicsContext::GetDevice()));
		
		vkDestroyDescriptorPool(
			GraphicsContext::GetDevice(),
			s_DescriptorPool,
			GraphicsContext::GetAllocator());

		vkDestroyCommandPool(
			GraphicsContext::GetDevice(),
			s_CommandPool,
			GraphicsContext::GetAllocator());

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		return true;
	}

	void GUIRenderer::BeginFrame()
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplVulkan_NewFrame();

		ImGui::NewFrame();
	}

	void GUIRenderer::EndFrame()
	{
		ImGui::Render();
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		Swapchain* swapchain = GraphicsContext::GetSwapchain();
		swapchain->RecordCommands([swapchain]() {
			const uint32_t frameIndex{ swapchain->GetImageIndex() };
			const VkExtent2D extent{ swapchain->GetExtent() };
			const VkCommandBuffer swapchainDrawCommandBuffer{ swapchain->GetCurrentCommandBuffer() };
			const VkFramebuffer swapchainDrawFramebuffer{ swapchain->GetCurrentFramebuffer() };
			const VkRenderPass swapchainDrawRenderPass{ swapchain->GetRenderPass() };

			constexpr std::array<VkClearValue, 1> clearValues{
				{ { 0.15f, 0.15f, 0.15f, 1.0f } }
			};

			constexpr VkCommandBufferBeginInfo drawCommandBufferBeginInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.pInheritanceInfo{ nullptr },
			};

			const VkRenderPassBeginInfo renderPassBeginInfo{
				.sType{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO },
				.pNext{ nullptr },
				.renderPass{ swapchainDrawRenderPass },
				.framebuffer{ swapchainDrawFramebuffer },
				.renderArea{ 
					.offset{ 0, 0 },
					.extent{ extent },
				},
				.clearValueCount{ 1U },
				.pClearValues{ &clearValues[0] },
			};

			const VkCommandBufferInheritanceInfo commandBufferInheritanceInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO },
				.pNext{ nullptr },
				.renderPass{ swapchainDrawRenderPass },
				.subpass{ 0U },
				.framebuffer{ swapchainDrawFramebuffer },
				.occlusionQueryEnable{ VK_FALSE },
				.queryFlags{ 0U },
				.pipelineStatistics{ 0U },
			};

			const VkCommandBufferBeginInfo imGuiCommandBufferBeginInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
				.pNext{ nullptr },
				.flags{ VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT },
				.pInheritanceInfo{ &commandBufferInheritanceInfo },
			};

			const VkViewport viewport{
				.x{ 0.0f },
				.y{ 0.0f },
				.width{ static_cast<float>(extent.width) },
				.height{ static_cast<float>(extent.height) },
				.minDepth{ 0.0f },
				.maxDepth{ 1.0f },
			};

			const VkRect2D scissor{
				.offset{ 0, 0 },
				.extent{ extent },
			};

			const VkCommandBuffer imguiCommandBuffer{ s_CommandBuffers[frameIndex] };
			VK_CHECK(vkBeginCommandBuffer(
				swapchainDrawCommandBuffer,
				&drawCommandBufferBeginInfo));

			vkCmdBeginRenderPass(
				swapchainDrawCommandBuffer,
				&renderPassBeginInfo,
				VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			VK_CHECK(vkBeginCommandBuffer(
				imguiCommandBuffer,
				&imGuiCommandBufferBeginInfo));

			vkCmdSetViewport(
				imguiCommandBuffer,
				0U,
				1U,
				&viewport);

			vkCmdSetScissor(
				imguiCommandBuffer,
				0U,
				1U,
				&scissor);

			ImDrawData* const drawData{ ImGui::GetDrawData() };
			ImGui_ImplVulkan_RenderDrawData(
				drawData,
				imguiCommandBuffer, 
				VK_NULL_HANDLE);

			VK_CHECK(vkEndCommandBuffer(
				imguiCommandBuffer));

			vkCmdExecuteCommands(
				swapchainDrawCommandBuffer,
				1U,
				&imguiCommandBuffer);

			vkCmdEndRenderPass(
				swapchainDrawCommandBuffer);

			VK_CHECK(vkEndCommandBuffer(
				swapchainDrawCommandBuffer));
		});
	}

	float GUIRenderer::GetFontSize()
	{
		return 17.0f;
	}

	float GUIRenderer::GetIconFontSize()
	{
		return 11.0f;
	}
	
	void GUIRenderer::UploadFontAtlas()
	{
		ImGuiIO& io{ ImGui::GetIO() };
		/* Regular font */
		io.Fonts->AddFontFromFileTTF("Resources/fonts/opensans/OpenSans-Regular.ttf", GetFontSize());
	}

	void GUIRenderer::UploadIconFontAtlas()
	{
		ImGuiIO& io{ ImGui::GetIO() };
		/* Icon font */
		InternalScope const ImWchar icon_ranges[]{ ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig config;
		config.OversampleH = config.OversampleV = 2;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = GetIconFontSize(); // Use if you want to make the icon monospaced

		io.Fonts->AddFontFromFileTTF("Resources/fonts/FontAwesome/fa-solid-900.ttf", GetIconFontSize(), &config, icon_ranges);
	}
}

#include "ThirdParty/imgui/backends/imgui_impl_vulkan.cpp"
#include "ThirdParty/imgui/backends/imgui_impl_win32.cpp"
#pragma warning(pop)
#endif