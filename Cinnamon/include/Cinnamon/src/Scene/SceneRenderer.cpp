#include "Cinnamon/include/Scene/SceneRenderer.hpp"
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Scene/Environment.hpp"
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
#include "Cinnamon/include/Renderer/Material.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

namespace Cinnamon {
	SceneRenderer::SceneRenderer(
		const STL::Unique<Renderer>& renderer,
		const bool swapchainTarget,
		const uint32_t viewportWidth,
		const uint32_t viewportHeight) noexcept
		:
		m_Renderer(renderer),
		m_SwapchainTarget(swapchainTarget),
		m_Framebuffer(nullptr),
		m_Renderer2D(nullptr),
		m_RenderedScene(nullptr),
		m_AspectRatio(static_cast<float>(viewportWidth) / viewportHeight)
	{
		const auto& swap{ m_Renderer->GetSwapchain() };
		(void)swap;
		FramebufferSpecification specification;
		specification.AttachmentSpecifications.push_back({ EImageFormat::R8G8B8A8, CinMath::Vector4{ 0.0f, 0.0f, 0.0f, 0.0f } });

		specification.ClearOnLoad = true;
		specification.Width = viewportWidth;
		specification.Height = viewportHeight;
		specification.Samples = 1;

		if (m_SwapchainTarget)
			m_Framebuffer = STL::MakeUnique<Framebuffer>(m_Renderer->GetAllocator(), std::move(specification), swap);
		else
			m_Framebuffer = STL::MakeUnique<Framebuffer>(m_Renderer->GetAllocator(), std::move(specification));
			//m_Framebuffer = STL::MakeUnique<Framebuffer>(m_Renderer->GetAllocator(), std::move(specification), swap);

		m_Renderer2D = STL::MakeUnique<Renderer2D>(m_Renderer, m_Renderer->GetAllocator(), m_Framebuffer);
		if(m_SwapchainTarget)
			m_RenderCommandBuffer = STL::MakeUnique<RenderCommandBuffer>(m_Renderer->GetDevice(), m_Renderer->GetSwapchain());
		else
			m_RenderCommandBuffer = STL::MakeUnique<RenderCommandBuffer>(m_Renderer->GetDevice(), m_Renderer->GetSwapchain()->GetImageCount());

		m_FullScreenQuadShader = STL::MakeUnique<Shader>
		(
			m_Renderer->GetAllocator(),
			"Resources/shaders/FullScreenQuad.shader",
			false
		);

		m_SkyboxPipeline = STL::MakeUnique<Pipeline>
		(
			m_Renderer->GetDevice(),
			m_Framebuffer,
			m_FullScreenQuadShader,
			VertexBufferLayout{},
			EPrimitiveTopology::Triangles
		);

		m_SkyBoxMaterial = STL::MakeUnique<Material>(m_FullScreenQuadShader);

		//m_Texture = cinew Texture2D("Resources/textures/paper.png", m_Renderer->GetAllocator(), {});
	}

	SceneRenderer::~SceneRenderer() noexcept
	{
		/* Resources could still be in used in the previous frames */
		VK_CHECK(vkDeviceWaitIdle(
			m_Renderer->GetDevice()->GetLogicalDevice()));
	}

	void SceneRenderer::RenderScene(const CinMath::Matrix4& camera)
	{
		const uint32_t frameIndex{ m_Renderer->GetSwapchain()->GetFrameIndex() };

		if (m_RenderedScene)
		{
			m_RenderCommandBuffer->Begin(frameIndex);
			m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_Framebuffer);
			m_Renderer->Clear(m_RenderCommandBuffer, m_Framebuffer);
			m_Renderer->EndRenderPass(m_RenderCommandBuffer);
			m_RenderCommandBuffer->End(frameIndex);
			m_RenderCommandBuffer->Submit(frameIndex);
			m_RenderCommandBuffer->Wait(frameIndex);
#if 0
			m_RenderedScene->GetEnvironment()->SetBackgroundTexture(m_Texture);

			if (Texture2D* const backgroundTexture{ m_RenderedScene->GetEnvironment()->GetBackgroundTexture() })
			{
				m_SkyBoxMaterial->SetTexture("u_Texture", backgroundTexture);
				
				m_RenderCommandBuffer->Begin(frameIndex);
				m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_Framebuffer);
				
				m_Renderer->Clear(m_RenderCommandBuffer, m_Framebuffer);
				
				m_Renderer->RenderFullscreenQuad
				(
					m_RenderCommandBuffer,
					m_SkyboxPipeline,
					m_SkyBoxMaterial
				);

				m_Renderer->EndRenderPass(m_RenderCommandBuffer);
				m_RenderCommandBuffer->End(frameIndex);
				m_RenderCommandBuffer->Submit(frameIndex);
				m_RenderCommandBuffer->Wait(frameIndex);
			}
#endif
			m_Renderer2D->BeginFrame(camera);
			
			for (const ECS::EntityID entityID : ECS::View<SpriteRendererComponent>(m_RenderedScene->GetRegistry()))
			{
				const auto& transform{ m_RenderedScene->GetRegistry()->Get<TransformComponent>(entityID) };
				const auto& sprite{ m_RenderedScene->GetRegistry()->Get<SpriteRendererComponent>(entityID) };
			
				if(sprite.Texture)
					m_Renderer2D->RenderQuad(transform.Calculate(), sprite.Color, sprite.TilingFactor, sprite.Texture);
			}
			
			for (const ECS::EntityID entityID : ECS::View<PointLightComponent>(m_RenderedScene->GetRegistry()))
			{
				const auto& transform{ m_RenderedScene->GetRegistry()->Get<TransformComponent>(entityID) };
				const auto& pointLight{ m_RenderedScene->GetRegistry()->Get<PointLightComponent>(entityID) };
			
				m_Renderer2D->RenderLight(transform.Translation, pointLight.Color, pointLight.Intensity);
			}
			
			m_Renderer2D->EndFrame(m_RenderCommandBuffer);	
		}
		else /* Clear the screen if no scene is found. */
		{
			m_RenderCommandBuffer->Begin(frameIndex);
			m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_Framebuffer);
			m_Renderer->Clear(m_RenderCommandBuffer, m_Framebuffer);
			m_Renderer->EndRenderPass(m_RenderCommandBuffer);
			m_RenderCommandBuffer->End(frameIndex);
			m_RenderCommandBuffer->Wait(frameIndex);
		}
	}

	void SceneRenderer::SetRenderedScene(Scene* const scene)
	{
		m_RenderedScene = scene;
	}

	void SceneRenderer::SetAspectRatio(const float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
	} 

	void SceneRenderer::SetViewportSize(const uint32_t viewportWidth, const uint32_t viewportHeight)
	{
		if (viewportWidth == 0U || viewportHeight == 0U)
			return;
		
		m_AspectRatio = static_cast<float>(viewportWidth) / viewportHeight;
		/* Resources could still be in used in the previous frame */
		VK_CHECK(vkDeviceWaitIdle(
			m_Renderer->GetDevice()->GetLogicalDevice()));

		m_Framebuffer->Invalidate(viewportWidth, viewportHeight);
		m_Renderer2D->SetViewportSize(viewportWidth, viewportHeight);
	}

	STL::Unique<Framebuffer>& SceneRenderer::GetFramebuffer() noexcept
	{
		return m_Framebuffer;
	}
}