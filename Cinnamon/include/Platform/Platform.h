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
		static [[nodiscard]] bool Initialize();
		static [[nodiscard]] bool Shutdown();

		static [[nodiscard]] double GetAbsoluteTime();
		/* Vulkan */
		static [[nodiscard]] STL::Vector<const char*> GetRequiredVulkanExtensions();
		static [[nodiscard]] STL::Vector<const char*> GetRequestedVulkanLayers();

		static [[nodiscard]] STL::Vector<const char*> GetRequiredVulkanDeviceExtensions();
		static [[nodiscard]] STL::Vector<const char*> GetRequestedVulkanDeviceLayers();
		
		static void WriteToConsole(const char* message, const EConsoleTextColor color);
	public:
	};
}