#pragma once
#include "Cinnamon/include/Core/Core.h"
#include <vector>
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

	class Platform
	{
	private:
		Platform() noexcept = delete;
		~Platform() noexcept = delete;
	public:
		static bool Initialize();
		static bool Shutdown();

		static double GetAbsoluteTime();

		/* Vulkan */
		static std::vector<const char*> GetRequiredVulkanExtensions();
		static std::vector<const char*> GetRequestedVulkanLayers();

		static std::vector<const char*> GetRequiredVulkanDeviceExtensions();
		static std::vector<const char*> GetRequestedVulkanDeviceLayers();
	public:
	};
}