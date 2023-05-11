#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include <functional>
namespace Cinnamon {
	class Window;
	class Device;
	class Swapchain;
	class VulkanAllocator;
	class VertexBuffer;
	class IndexBuffer;
	class Shader;
	class Pipeline;
	class Framebuffer;
	class RenderCommandBuffer;
	class DescriptorPool;

	class Renderer final
	{
	private:
		NON_COPYABLE(Renderer)
	public:
		explicit Renderer(const STL::Unique<Window>& windowContext) noexcept;
		~Renderer() noexcept;

		void BeginFrame();
		void EndFrame();

		void BeginRenderPass(
			const STL::Unique<RenderCommandBuffer>& renderCommandBuffer,
			const STL::Unique<Framebuffer>& framebuffer);
			
		void EndRenderPass(
			const STL::Unique<RenderCommandBuffer>& renderCommandBuffer);

		void RenderGeometry(
			const STL::Unique<RenderCommandBuffer>& renderCommandBuffer,
			const STL::Unique<VertexBuffer>& vertexBuffer,
			const STL::Unique<IndexBuffer>& indexBuffer,
			const STL::Unique<Pipeline>& pipeline,
			const uint32_t indexCount);

		void SetClearColor(
			const float r, 
			const float g, 
			const float b, 
			const float a);

		void SetViewportSize(
			const uint32_t width, 
			const uint32_t height);

		[[nodiscard]] uint32_t
			GetFrameIndex() const;

		[[nodiscard]] const STL::Unique<Device>& 
			GetDevice() const;

		[[nodiscard]] const STL::Unique<Swapchain>& 
			GetSwapchain() const;

		[[nodiscard]] const STL::Unique<DescriptorPool>&
			GetDescriptorPool() const;
	private:
		STL::Unique<Device>				m_Device;
		STL::Unique<Swapchain>			m_Swapchain;
		STL::Unique<VulkanAllocator>	m_Allocator;
		STL::Unique<DescriptorPool>		m_DescriptorPool;
	};
}