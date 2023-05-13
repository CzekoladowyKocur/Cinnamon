#include "Cinnamon/include/Scene/SceneRenderer.hpp"
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/ECS/Registry.hpp"

#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"

#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/IndexBuffer.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/Renderer2D.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

namespace Cinnamon {
	SceneRenderer::SceneRenderer(
		const STL::Unique<Renderer>& renderer,
		const uint32_t viewportWidth,
		const uint32_t viewportHeight) noexcept
		:
		m_RenderedScene(nullptr),
		m_Renderer(renderer),
		m_Allocator(STL::MakeUnique<VulkanAllocator>(m_Renderer->GetDevice())),
		m_Framebuffer(STL::MakeUnique<Framebuffer>(m_Allocator, FramebufferSpecification{ viewportWidth, viewportHeight, 1U, EImageFormat::R8G8B8A8 })),
		m_Renderer2D(STL::MakeUnique<Renderer2D>(m_Renderer, m_Allocator, m_Framebuffer))
	{}

	SceneRenderer::~SceneRenderer() noexcept
	{
		/* Resources could still be in used in the previous frames */
		VK_CHECK(vkDeviceWaitIdle(
			m_Renderer->GetDevice()->GetLogicalDevice()));
	}

	void SceneRenderer::BeginFrame()
	{
		if (m_RenderedScene)
		{
			m_Renderer2D->BeginFrame();
			for (const ECS::EntityID entityID : ECS::View<TransformComponent>(m_RenderedScene->GetRegistry()))
			{
				const auto& transform{ m_RenderedScene->GetRegistry()->Get<TransformComponent>(entityID) };
				
				m_Renderer2D->RenderQuad(transform.Calculate());
			}
			m_Renderer2D->EndFrame();
		}
	}

	void SceneRenderer::EndFrame()
	{}

	void SceneRenderer::SetRenderedScene(const Scene* const scene)
	{
		m_RenderedScene = scene;
	}

	void SceneRenderer::SetViewportSize(const uint32_t viewportWidth, const uint32_t viewportHeight)
	{
		if (viewportWidth == 0U || viewportHeight == 0U)
			return;
		
		/* Resources could still be in used in the previous frame */
		VK_CHECK(vkDeviceWaitIdle(
			m_Renderer->GetDevice()->GetLogicalDevice()));

		m_Framebuffer->Invalidate(viewportWidth, viewportHeight);
	}

	STL::Unique<Framebuffer>& SceneRenderer::GetFramebuffer() noexcept
	{
		return m_Framebuffer;
	}
}