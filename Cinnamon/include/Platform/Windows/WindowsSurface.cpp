#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"

namespace Cinnamon {
	struct PlatformWindowState
	{
		HWND Handle{ nullptr };
		HINSTANCE Instance{ nullptr };

		uint32_t StyleFlags{ 0U };
		uint32_t ExtendedStyleFlags{ 0U };
	};

	namespace Platform
	{
		/* Surface -> [Window context, desired present mode] */
		InternalScope STL::UMap<VkSurfaceKHR, std::pair<const PlatformWindowState*, VkPresentModeKHR>> s_SurfaceMap;

		VkSurfaceKHR CreateWindowSurface(const STL::Unique<Window>& window)
		{
			CIN_ASSERT(window);
			const PlatformWindowState* const windowState{ window->GetState() };
			
			const VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo
			{
				.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
				.pNext{ nullptr },
				.flags{ 0U },
				.hinstance{ windowState->Instance },
				.hwnd{ windowState->Handle },
			};

			VkSurfaceKHR surface{ VK_NULL_HANDLE };
			VK_CHECK(vkCreateWin32SurfaceKHR(
				GraphicsContext::GetInstance(),
				&win32SurfaceCreateInfo,
				GraphicsContext::GetAllocator(),
				&surface));

			const WindowProperties& windowProperties{ window->GetProperties() };
			CIN_ASSERT(!s_SurfaceMap.contains(surface));
			s_SurfaceMap[surface] = std::make_pair(windowState, windowProperties.UseVSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_MAILBOX_KHR);

			return surface;
		}

		VkSurfaceKHR RetrieveWindowSurface(const STL::Unique<Window>& window)
		{
			/* Check if surface was already created by the device */
			for (const auto& [surface, surfaceState] : s_SurfaceMap)
			{
				const auto& [windowState, desiredPresentMode] { surfaceState};

				if (window->GetState() == windowState)
					return surface;
			}

			return CreateWindowSurface(window);
		}

		VkSurfaceKHR RecreateWindowSurface(const VkSurfaceKHR surface)
		{
			CIN_ASSERT(s_SurfaceMap.contains(surface));
			
			vkDestroySurfaceKHR(
				GraphicsContext::GetInstance(),
				surface,
				GraphicsContext::GetAllocator());

			const PlatformWindowState* const windowState{ s_SurfaceMap[surface].first };
			const VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo
			{
				.sType{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR },
				.pNext{ nullptr },
				.flags{ 0U },
				.hinstance{ windowState->Instance },
				.hwnd{ windowState->Handle },
			};

			VkSurfaceKHR newSurface{ VK_NULL_HANDLE };
			VK_CHECK(vkCreateWin32SurfaceKHR(
				GraphicsContext::GetInstance(),
				&win32SurfaceCreateInfo,
				GraphicsContext::GetAllocator(),
				&newSurface));

			return newSurface;
		}

		VkPresentModeKHR GetDesiredSurfacePresentMode(const VkSurfaceKHR surface)
		{
			CIN_ASSERT(s_SurfaceMap.contains(surface));
			return s_SurfaceMap[surface].second;
		}

		void DestroySurface(const VkSurfaceKHR surface)
		{
			CIN_ASSERT(s_SurfaceMap.contains(surface));

			vkDestroySurfaceKHR(
				GraphicsContext::GetInstance(),
				surface,
				GraphicsContext::GetAllocator());

			s_SurfaceMap.erase(surface);
		}
	}
}
#endif