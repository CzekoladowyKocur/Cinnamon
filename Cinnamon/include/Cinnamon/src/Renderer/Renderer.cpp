#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Core/Window.hpp"

namespace Cinnamon {
	Renderer::Renderer(const STL::Unique<Window>& windowContext)
		:
		m_Window(windowContext),
		m_Surface(STL::MakeUnique<Surface>(windowContext)),
		m_Device(STL::MakeUnique<Device>(m_Surface)),
		m_Swapchain(STL::MakeUnique<Swapchain>(m_Surface, m_Device, windowContext->GetWidth(), windowContext->GetHeight()))
	{}

	Renderer::~Renderer()
	{
		m_Swapchain.reset();
		m_Device.reset();
		m_Surface.reset();
	}

	void Renderer::BeginFrame()
	{
		m_Swapchain->AcquireNextSwapchainImage();
	}

	void Renderer::EndFrame()
	{
		m_Swapchain->PresentSwapchainImage();
	}

	void Renderer::SetClearColor(
		const float r, 
		const float g, 
		const float b, 
		const float a)
	{
		m_Swapchain->SetClearColor(r, g, b, a);
	}

	void Renderer::SetViewportSize(
		const uint32_t width, 
		const uint32_t height)
	{
		m_Swapchain->Recreate(width, height);
	}

	const STL::Unique<Window>& Renderer::GetWindow()
	{
		return m_Window;
	}

	const STL::Unique<Surface>& Renderer::GetSurface()
	{
		return m_Surface;
	}

	const STL::Unique<Device>& Renderer::GetDevice()
	{
		return m_Device;
	}

	const STL::Unique<Swapchain>& Renderer::GetSwapchain()
	{
		return m_Swapchain;
	}
}