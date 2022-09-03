#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.h"
#include "Cinnamon/include/Core/Window.h"

namespace Cinnamon {
	class Surface
	{
	private:
	public:
		explicit Surface(const Window* const windowContext) noexcept;
		~Surface() noexcept;

		VkPresentModeKHR GetDesiredPresentMode() const;
		VkSurfaceKHR GetHandle() const;
	private:
		const PlatformWindowState* m_WindowState;
		bool m_UseVSync;

		VkPresentModeKHR m_DesiredPresentMode;
		VkSurfaceKHR m_Handle;
	};
}