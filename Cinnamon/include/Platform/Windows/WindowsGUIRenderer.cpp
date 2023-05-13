#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/GUI/GUIRenderer.hpp"
#include "Cinnamon/include/GUI/Icons.hpp"
#include "Cinnamon/include/GUI/GUIUtilities.hpp"
#include "Cinnamon/include/GUI/GUIThemes.hpp"

#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Cinnamon/include/Core/KeyCodes.hpp"
#include "Cinnamon/include/Event/Event.hpp"
#include "Cinnamon/include/Event/KeyEvent.hpp"
#include "Cinnamon/include/Event/MouseEvent.hpp"

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 4616)
#pragma warning(disable : 4100)
#pragma warning(disable : 4211)
#pragma warning(disable : 6011)
#pragma warning(disable : 33010)
#pragma warning(disable : 28182)
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/backends/imgui_impl_vulkan.h"
#include "ThirdParty/imgui/backends/imgui_impl_win32.h"

namespace Cinnamon {
	struct InternalGUIRendererState
	{
		ImGuiContext*					Context{ nullptr };
		/* Vulkan context */
		VkDescriptorPool				DescriptorPool{ VK_NULL_HANDLE };
	};

	GUIRenderer::GUIRenderer(
		const STL::Unique<Window>& window,
		const STL::Unique<Renderer>& renderer) noexcept
		:
		m_Renderer(renderer),
		m_InternalState(cinew InternalGUIRendererState)
	{
		CIN_ASSERT(renderer);
		IMGUI_CHECKVERSION();

		if (not (m_InternalState->Context = ImGui::CreateContext()))
			CIN_PANIC_EXIT();

		ImGui::SetCurrentContext(m_InternalState->Context);

		ImGuiIO& io{ ImGui::GetIO() };
		ImGuiStyle& style{ ImGui::GetStyle() };
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		CIN_UNUSED(style);

		SetTheme(EUITheme::Default);
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

		const auto& device{ renderer->GetDevice() };
		const auto& swapchain{ renderer->GetSwapchain() };

		VK_CHECK(vkCreateDescriptorPool(
			device->GetLogicalDevice(),
			&descriptorPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_InternalState->DescriptorPool));

		ImGui_ImplVulkan_InitInfo vulkanInitializeInfo
		{
			.Instance{ GraphicsContext::GetInstance() },
			.PhysicalDevice{ device->GetPhysicalDevice() },
			.Device{ device->GetLogicalDevice() },
			.QueueFamily{ device->GetQueueFamilies().Graphics },
			.Queue{ device->GetQueues().Graphics },
			.PipelineCache{ VK_NULL_HANDLE },
			.DescriptorPool{ m_InternalState->DescriptorPool },
			.Subpass{ 0U },
			.MinImageCount{ swapchain->GetImageMinimalCount() },
			.ImageCount{ swapchain->GetImageCount() },
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

		CIN_VERIFY(ImGui_ImplWin32_Init(const_cast<void*>(window->GetNativeHandle())));

		CIN_VERIFY(ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* userData)
		{
			return vkGetInstanceProcAddr(
				GraphicsContext::GetInstance(),
				function_name);
		}));

		ImGui::GetPlatformIO().Platform_CreateVkSurface = [](ImGuiViewport* const viewport, const ImU64 vulkanInstance, const void* allocator, ImU64* outVulkanSurface)
		{
			const VkWin32SurfaceCreateInfoKHR createInfo{
				.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR} ,
				.pNext{ nullptr },
				.flags{ 0U },
				.hinstance{ GetModuleHandle(NULL) },
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
			swapchain->GetRenderPass()));

		UploadFontAtlas();
		UploadIconFontAtlas();

		device->PerformSingleSubmitGraphicsOperation([](const VkCommandBuffer fontCommandBuffer)
		{
			ImGui_ImplVulkan_CreateFontsTexture(fontCommandBuffer);
		});

		ImGui_ImplVulkan_DestroyFontUploadObjects();
		const VkCommandPoolCreateInfo commandPoolCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
			.pNext{ nullptr },
			.flags
			{
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT /* Command buffers will be short lived */
			},
			.queueFamilyIndex{ device->GetQueueFamilies().Graphics },
		};
	}

	GUIRenderer::~GUIRenderer() noexcept
	{
		auto& device{ m_Renderer->GetDevice() };

		VK_CHECK(vkDeviceWaitIdle(
			device->GetLogicalDevice()));

		vkDestroyDescriptorPool(
			device->GetLogicalDevice(),
			m_InternalState->DescriptorPool,
			GraphicsContext::GetAllocator());

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext(m_InternalState->Context);

		cindel m_InternalState;
	}

	void GUIRenderer::OnEvent(const Event& event)
	{
		ImGui::SetCurrentContext(m_InternalState->Context);
		ImGuiIO& IO{ ImGui::GetIO() };

		/* For mouse code and key code conversions */
		using namespace GUIUtilities;
		switch (event.GetEventType())
		{
			case EEventType::KeyPressed:
			{
				const KeyPressedEvent& keyPressedEvent{ static_cast<const KeyPressedEvent&>(event) };
				IO.AddKeyEvent(NativeKeyCodeToImGUIKeyCode(keyPressedEvent.GetKeyCode()), true);
				
				const char inputCharacter{ NativeKeyCodeToToChar(keyPressedEvent.GetKeyCode()) };

				if (ImGui::IsKeyDown(ImGuiKey_RightShift) || ImGui::IsKeyDown(ImGuiKey_LeftShift) || (GetKeyState(VK_CAPITAL) & 0x0001))
					IO.AddInputCharacter(inputCharacter);
				else
					IO.AddInputCharacter(static_cast<char>(std::tolower(static_cast<char>(inputCharacter))));
			} break;

			case EEventType::KeyReleased:
			{
				const KeyReleasedEvent& keyReleasedEvent{ static_cast<const KeyReleasedEvent&>(event) };
				IO.AddKeyEvent(NativeKeyCodeToImGUIKeyCode(keyReleasedEvent.GetKeyCode()), false);
			} break;

			case EEventType::MousePressed:
			{
				const MousePressedEvent& mousePressedEvent{ static_cast<const MousePressedEvent&>(event) };
				IO.AddMouseButtonEvent(NativeMouseCodeToImGUIMouseCode(mousePressedEvent.GetMouseCode()), true);
			} break;

			case EEventType::MouseReleased:
			{
				const MouseReleasedEvent& mouseReleasedEvent{ static_cast<const MouseReleasedEvent&>(event) };
				IO.AddMouseButtonEvent(NativeMouseCodeToImGUIMouseCode(mouseReleasedEvent.GetMouseCode()), false);
			} break;

			default: break;
		}
	}

	void GUIRenderer::BeginFrame()
	{
		ImGui::SetCurrentContext(m_InternalState->Context);
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

		const auto& swapchain{ m_Renderer->GetSwapchain() };	
		swapchain->RecordCommands([this]
		(
			const VkCommandBuffer commandBuffer, 
			const VkFramebuffer frameBuffer,
			const VkRenderPass renderPass,
			const VkExtent2D extent
		) 
		{
			constexpr std::array<VkClearValue, 1> clearValues{ { { 0.15f, 0.15f, 0.15f, 1.0f } } };
			constexpr VkCommandBufferBeginInfo drawCommandBufferBeginInfo
			{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.pInheritanceInfo{ nullptr },
			};

			const VkRenderPassBeginInfo renderPassBeginInfo
			{
				.sType{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO },
				.pNext{ nullptr },
				.renderPass{ renderPass },
				.framebuffer{ frameBuffer },
				.renderArea
				{
					.offset{ 0, 0 },
					.extent{ extent },
				},
				.clearValueCount{ 1U },
				.pClearValues{ &clearValues[0] },
			};

			const VkCommandBufferBeginInfo imGuiCommandBufferBeginInfo
			{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
				.pNext{ nullptr },
				.flags{ VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT },
				.pInheritanceInfo{ nullptr },
			};

			const VkViewport viewport
			{
				.x{ 0.0f },
				.y{ 0.0f },
				.width{ static_cast<float>(extent.width) },
				.height{ static_cast<float>(extent.height) },
				.minDepth{ 0.0f },
				.maxDepth{ 1.0f },
			};

			const VkRect2D scissor
			{
				.offset{ 0, 0 },
				.extent{ extent },
			};

			VK_CHECK(vkBeginCommandBuffer(
				commandBuffer,
				&drawCommandBufferBeginInfo));

			vkCmdBeginRenderPass(
				commandBuffer,
				&renderPassBeginInfo,
				VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(
				commandBuffer,
				0U,
				1U,
				&viewport);

			vkCmdSetScissor(
				commandBuffer,
				0U,
				1U,
				&scissor);

			ImDrawData* const drawData{ ImGui::GetDrawData() };
			ImGui_ImplVulkan_RenderDrawData(
				drawData,
				commandBuffer,
				VK_NULL_HANDLE);

			vkCmdEndRenderPass(
				commandBuffer);

			VK_CHECK(vkEndCommandBuffer(
				commandBuffer));
		});

		//ImGui::SetCurrentContext(nullptr);
	}

	void GUIRenderer::SetTheme(const EUITheme theme)
	{
		ImGui::SetCurrentContext(m_InternalState->Context);
		SetUITheme(ImGui::GetStyle(), theme);
	}

	void GUIRenderer::UploadFontAtlas()
	{
		ImGui::SetCurrentContext(m_InternalState->Context);
		ImGuiIO& io{ ImGui::GetIO() };
		/* Regular font */
		io.Fonts->AddFontFromFileTTF("Resources/fonts/opensans/OpenSans-Regular.ttf", GUIUtilities::GetRegularFontSize());
	}

	void GUIRenderer::UploadIconFontAtlas()
	{
		ImGui::SetCurrentContext(m_InternalState->Context);
		ImGuiIO& io{ ImGui::GetIO() };
		/* Icon font */
		InternalScope const ImWchar icon_ranges[]{ ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig config;
		config.OversampleH = config.OversampleV = 2;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = GUIUtilities::GetIconFontSize(); // Use if you want to make the icon monospaced

		io.Fonts->AddFontFromFileTTF("Resources/fonts/FontAwesome/fa-solid-900.ttf", GUIUtilities::GetIconFontSize(), &config, icon_ranges);
	}
}

#include "ThirdParty/imgui/backends/imgui_impl_win32.cpp"
#pragma warning(pop)
#endif