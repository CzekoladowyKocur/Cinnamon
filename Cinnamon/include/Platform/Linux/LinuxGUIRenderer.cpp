#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/TypeDefines.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Cinnamon/include/GUI/GUIRenderer.hpp"
#include "Cinnamon/include/GUI/Icons.hpp"
#include "Cinnamon/include/GUI/GUIUtilities.hpp"
#include "Cinnamon/include/GUI/GUIThemes.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Cinnamon/include/Core/Input.hpp"
#include "Cinnamon/include/Event/KeyEvent.hpp"
#include "Cinnamon/include/Event/MouseEvent.hpp"

#include <linux/uinput.h>
#include "ThirdParty/imgui/imgui_internal.h"
#include "ThirdParty/imgui/imgui.h"
#define VK_NO_PROTOTYPES
#include "ThirdParty/imgui/backends/imgui_impl_vulkan.h"
//#include "ThirdParty/imgui/backends/imgui_impl_win32.h"

namespace Cinnamon {
	struct LinuxBackendData
	{
		const Window* WindowHandle{ nullptr };
		ImGuiMouseCursor LastMouseCursor{};
	};

	struct InternalGUIRendererState
	{
		ImGuiContext* 					Context{ nullptr };
		LinuxBackendData*				Backend{ nullptr };

		/* Vulkan context */
		VkDescriptorPool				DescriptorPool{ VK_NULL_HANDLE };
		VkCommandPool					CommandPool{ VK_NULL_HANDLE };
		STL::Vector<VkCommandBuffer>	CommandBuffers{};
	};

	static LinuxBackendData* LinuxBackendInitialize(const Window* const window)
	{
		ImGuiIO& io { ImGui::GetIO() };
		IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend");

		LinuxBackendData* backendData{ cinew LinuxBackendData() };
		backendData->WindowHandle = window;
		backendData->LastMouseCursor = {};

		io.BackendPlatformUserData = reinterpret_cast<void*>(backendData);
		io.BackendPlatformName = "Linux backend";
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
		//io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;

		return backendData;
	}

	static void LinuxBackendShutdown(LinuxBackendData* backendData)
	{
		CIN_ASSERT(backendData)
		delete backendData;
	}

	static bool LinuxBackendUpdateMouseCursor(LinuxBackendData* backendData)
	{
		CIN_UNUSED(backendData);

		ImGuiIO& io { ImGui::GetIO() };
    	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
    	    return false;

		return true;
	}

	static void LinuxBackendNewFrame(LinuxBackendData* backendData)
	{
		const Window* window{ backendData->WindowHandle };
		const auto[width, height]{ window->GetSize() };
		
		ImGuiIO& io { ImGui::GetIO() };
		io.DisplaySize = ImVec2
		(
			static_cast<float>(width),
			static_cast<float>(height)
		);

		[[likely]]
		if(window->GetProperties().Focused)
		{
			const auto [xPosition, yPosition]{ Input::GetMousePosition() };
			io.AddMousePosEvent
			(
				static_cast<float>(xPosition),
				static_cast<float>(yPosition)
			);			
		}

		ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    	if (backendData->LastMouseCursor != mouse_cursor)
    	{
    	    backendData->LastMouseCursor = mouse_cursor;
			LinuxBackendUpdateMouseCursor(backendData);
    	    //ImGui_ImplWin32_UpdateMouseCursor();
    	}
	}
	
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

		const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
		{
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
			.MinImageCount{ 3 },
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

		CIN_VERIFY(ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* userData)
		{
			CIN_UNUSED(userData);
			return vkGetInstanceProcAddr(
				GraphicsContext::GetInstance(),
				function_name);
		}));

		m_InternalState->Backend = LinuxBackendInitialize(const_cast<Window*>(window.get()));

		CIN_VERIFY(ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* userData)
			{
				CIN_UNUSED(userData);
				return vkGetInstanceProcAddr(
					GraphicsContext::GetInstance(),
					function_name);	
			}));

		CIN_VERIFY(ImGui_ImplVulkan_Init(
			&vulkanInitializeInfo,
			swapchain->GetRenderPass()));

		UploadFontAtlas();
		UploadIconFontAtlas();

		device->PerformSingleSubmitGraphicsOperation([](VkCommandBuffer fontCommandBuffer)
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

		VK_CHECK(vkCreateCommandPool(
			device->GetLogicalDevice(),
			&commandPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_InternalState->CommandPool));

		const VkCommandBufferAllocateInfo imGuiCommandBufferAllocateInfo{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
			.pNext{ nullptr },
			.commandPool{ m_InternalState->CommandPool },
			.level{ VK_COMMAND_BUFFER_LEVEL_SECONDARY },
			.commandBufferCount{ swapchain->GetImageCount() },
		};

		m_InternalState->CommandBuffers.resize(swapchain->GetImageCount());
		VK_CHECK(vkAllocateCommandBuffers(
			device->GetLogicalDevice(),
			&imGuiCommandBufferAllocateInfo,
			&m_InternalState->CommandBuffers[0]));
	}

	GUIRenderer::~GUIRenderer()
	{
		auto& device{ m_Renderer->GetDevice() };

		VK_CHECK(vkDeviceWaitIdle(
			device->GetLogicalDevice()));

		vkDestroyDescriptorPool(
			device->GetLogicalDevice(),
			m_InternalState->DescriptorPool,
			GraphicsContext::GetAllocator());

		vkDestroyCommandPool(
			device->GetLogicalDevice(),
			m_InternalState->CommandPool,
			GraphicsContext::GetAllocator());

		ImGui_ImplVulkan_Shutdown();
		LinuxBackendShutdown(m_InternalState->Backend);
		ImGui::DestroyContext(m_InternalState->Context);
		
		cindel m_InternalState;
	}

	void GUIRenderer::BeginFrame()
	{
		LinuxBackendNewFrame(m_InternalState->Backend);
		ImGui_ImplVulkan_NewFrame();

		ImGui::NewFrame();
	}

	void GUIRenderer::EndFrame()
	{
		ImGui::Render();
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
	
	void GUIRenderer::UploadFontAtlas()
	{
		ImGuiIO& io{ ImGui::GetIO() };
		/* Regular font */
		CIN_TRACE("Loading regular font: Resources/fonts/opensans/OpenSans-Regular.ttf");
		io.Fonts->AddFontFromFileTTF("Resources/fonts/opensans/OpenSans-Regular.ttf", GUIUtilities::GetRegularFontSize());
	}

	void GUIRenderer::UploadIconFontAtlas()
	{
		ImGuiIO& io{ ImGui::GetIO() };
		/* Icon font */
		InternalScope const ImWchar icon_ranges[]{ ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig config;
		config.OversampleH = config.OversampleV = 2;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = GUIUtilities::GetIconFontSize(); // Use if you want to make the icon monospaced

		CIN_TRACE("Loading icon font: Resources/fonts/FontAwesome/fa-solid-900.ttf");
		io.Fonts->AddFontFromFileTTF("Resources/fonts/FontAwesome/fa-solid-900.ttf", GUIUtilities::GetIconFontSize(), &config, icon_ranges);
	}
}

#endif