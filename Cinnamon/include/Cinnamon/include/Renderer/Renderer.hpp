#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Window;
	class Device;
	class Surface;
	class Swapchain;

	class Renderer final
	{
	private:
		NON_COPYABLE(Renderer)
	public:
		explicit Renderer(const STL::Unique<Window>& windowContext);
		~Renderer();

		void BeginFrame();
		void EndFrame();

		void SetClearColor(
			const float r, 
			const float g, 
			const float b, 
			const float a);

		void SetViewportSize(
			const uint32_t width, 
			const uint32_t height);

		const STL::Unique<Window>& GetWindow();
		const STL::Unique<Surface>& GetSurface();
		const STL::Unique<Device>& GetDevice();
		const STL::Unique<Swapchain>& GetSwapchain();
	private:
		const STL::Unique<Window>&	m_Window;
		STL::Unique<Surface>		m_Surface;
		STL::Unique<Device>			m_Device;
		STL::Unique<Swapchain>		m_Swapchain;
	};
}