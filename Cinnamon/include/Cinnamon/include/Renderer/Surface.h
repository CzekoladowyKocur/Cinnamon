#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.h"
#include "Cinnamon/include/Core/Window.h"

namespace Cinnamon {
	class Surface
	{
	private:
	public:
		explicit Surface(const Window* windowContext) noexcept;
		~Surface() noexcept;

		VkSurfaceKHR GetHandle() const;
	private:
		const PlatformWindowState* m_WindowState;
		VkSurfaceKHR m_Handle;
	};
}