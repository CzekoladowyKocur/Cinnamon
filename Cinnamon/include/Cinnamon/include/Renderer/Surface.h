#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.h"
#include "Cinnamon/include/Core/Window.h"

namespace Cinnamon {
	class Surface
	{
	private:
		NON_COPYABLE(Surface)
	public:
		explicit Surface(const STL::Unique<Window>& windowContext) noexcept;
		~Surface() noexcept;

		void Recreate();
		VkPresentModeKHR GetDesiredPresentMode() const;
		VkSurfaceKHR GetHandle() const;
	private:
		const PlatformWindowState* m_WindowState;
		bool m_UseVSync;

		VkPresentModeKHR m_DesiredPresentMode;
		VkSurfaceKHR m_Handle;
	};
}