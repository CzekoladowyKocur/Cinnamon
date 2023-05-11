#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"
#include "Cinnamon/include/Core/Window.hpp"

namespace Cinnamon {
	class Window;

	namespace Platform
	{
		[[nodiscard]] VkSurfaceKHR CreateWindowSurface(const STL::Unique<Window>& window);
		[[nodiscard]] VkSurfaceKHR RetrieveWindowSurface(const STL::Unique<Window>& window);
		[[nodiscard]] VkSurfaceKHR RecreateWindowSurface(const VkSurfaceKHR surface);
		[[nodiscard]] VkPresentModeKHR GetDesiredSurfacePresentMode(const VkSurfaceKHR surface);
		void DestroySurface(const VkSurfaceKHR surface);
	}

	//class Surface
	//{
	//private:
	//	NON_COPYABLE(Surface)
	//public:
	//	explicit Surface(const STL::Unique<Window>& windowContext) noexcept;
	//	~Surface() noexcept;
	//
	//	void Recreate();
	//	VkPresentModeKHR GetDesiredPresentMode() const;
	//	VkSurfaceKHR GetHandle() const;
	//private:
	//	const PlatformWindowState* m_WindowState;
	//	bool m_UseVSync;
	//
	//	VkPresentModeKHR m_DesiredPresentMode;
	//	VkSurfaceKHR m_Handle;
	//};
}