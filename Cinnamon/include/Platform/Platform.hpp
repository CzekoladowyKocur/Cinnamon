#pragma once
#include "Cinnamon/include/Core/TypeDefines.hpp"
#include "Cinnamon/include/Core/CinSTL.hpp"

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
		Linux	= BIT(2U),
		MacOS	= BIT(3U),
		IOS		= BIT(4U),
		Android = BIT(5U)
	};

	enum class EConsoleTextColor
	{
		White	= 0,
		Gray	= 1,
		Yellow	= 2,
		Red		= 3,
		Magenta = 4,
	};

	namespace Platform 
	{
		Errr Initialize();
		void Shutdown();

		void									WriteToConsole(const char* message, const EConsoleTextColor color);
		[[nodiscard]] double					GetAbsoluteTime();
		[[nodiscard]] STL::String				GetBuildDate();
		[[nodiscard]] STL::String				GenerateUUID();
		[[nodiscard]] STL::Vector<const char*>	GetRequiredVulkanExtensions();
		[[nodiscard]] STL::Vector<const char*>	GetRequestedVulkanLayers();
		[[nodiscard]] STL::Vector<const char*>	GetRequiredVulkanDeviceExtensions();
		[[nodiscard]] STL::Vector<const char*>	GetRequestedVulkanDeviceLayers();
	}
}