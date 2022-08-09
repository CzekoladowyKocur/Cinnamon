#pragma once
#include "Cinnamon/include/Core/TypeDefines.h"
#include "Cinnamon/include/Core/CinSTL.h"

namespace Cinnamon {
	enum class EBuildMode
	{
		Debug,
		Release,
		Distribtuon,
	};

	enum class EPlatformFlags
	{
		Windows = BIT(1U),
		Linux = BIT(2U),
		MacOS = BIT(3U),
		IOS = BIT(4U),
		Android = BIT(5U)
	};

	enum class EConsoleTextColor
	{
		White = 0,
		Gray = 1,
		Yellow = 2,
		Red = 3,
		Magenta = 4,
	};

	class Platform
	{
	private:
		Platform() noexcept = delete;
		~Platform() noexcept = delete;
	public:
		[[nodiscard]] static bool Initialize();
		[[nodiscard]] static bool Shutdown();

		[[nodiscard]] static double GetAbsoluteTime();
		/* Vulkan */
		[[nodiscard]] static STL::Vector<const char*> GetRequiredVulkanExtensions();
		[[nodiscard]] static STL::Vector<const char*> GetRequestedVulkanLayers();

		[[nodiscard]] static STL::Vector<const char*> GetRequiredVulkanDeviceExtensions();
		[[nodiscard]] static STL::Vector<const char*> GetRequestedVulkanDeviceLayers();

		[[nodiscard]] static STL::String GetBuildDate();
		
		static void WriteToConsole(const char* message, const EConsoleTextColor color);
	public:
	};
}