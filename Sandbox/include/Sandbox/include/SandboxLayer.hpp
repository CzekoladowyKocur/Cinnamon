#pragma once
#include "Cinnamon/include/Core/Layer.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/IndexBuffer.hpp"

using namespace Cinnamon;

namespace Cinnamon {
}

class SandboxLayer final : public Layer
{
private:
	NON_COPYABLE(SandboxLayer)
public:
	SandboxLayer(STL::Unique<Window>& window, STL::Unique<Renderer>& renderer) noexcept;
	virtual ~SandboxLayer() noexcept;

	virtual void OnAttach() override final;
	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnDetach() override final;

	virtual void OnEvent(const Event& event) override final;
private:
	STL::Unique<Window>& m_Window;
	STL::Unique<Renderer>& m_Renderer;
	STL::Unique<VulkanAllocator> m_Allocator;
	STL::Unique<Framebuffer> m_Framebuffer;
	STL::Unique<RenderCommandBuffer> m_RenderCommandBuffer;

	STL::Unique<VertexBuffer>		m_QuadVertexBuffer;
	STL::Unique<IndexBuffer>		m_QuadIndexBuffer;
	STL::Unique<Shader>				m_QuadShader;
	STL::Unique<Pipeline>			m_Pipeline;
};
