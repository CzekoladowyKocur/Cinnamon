#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Scene;
	class Renderer;
	class Renderer2D; 
	class Framebuffer;
	class RenderCommandBuffer;	
	class VulkanAllocator;

	class SceneRenderer final
	{
	private:
		NON_COPYABLE(SceneRenderer)
	public:
		explicit SceneRenderer(
			const STL::Unique<Renderer>& renderer,
			const uint32_t viewportWidth,
			const uint32_t viewportHeight) noexcept;

		~SceneRenderer() noexcept;

		void BeginFrame();
		void EndFrame();
		void SetRenderedScene(const Scene* const scene);
		void SetViewportSize(const uint32_t viewportWidth, const uint32_t viewportHeight);

		[[nodiscard]] STL::Unique<Framebuffer>& 
			GetFramebuffer() noexcept;
	private:
		const Scene* m_RenderedScene;
		const STL::Unique<Renderer>& m_Renderer;

		STL::Unique<VulkanAllocator> m_Allocator;
		STL::Unique<Framebuffer> m_Framebuffer;
		STL::Unique<Renderer2D> m_Renderer2D;
	};
}