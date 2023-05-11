#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
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
		void SetViewportSize(const uint32_t viewportWidth, const uint32_t viewportHeight);

		[[nodiscard]] STL::Unique<Framebuffer>& 
			GetFramebuffer() noexcept;
	private:
		const STL::Unique<Renderer>& m_Renderer;

		STL::Unique<VulkanAllocator> m_Allocator;
		STL::Unique<Framebuffer> m_Framebuffer;
		STL::Unique<Renderer2D> m_Renderer2D;

		//STL::Unique<RenderCommandBuffer> m_RenderCommandBuffer;
	};
}