#pragma once
#include "Cinnamon/include/Core/Core.h"

/* From vulkan.h */
struct VkAllocationCallbacks;
struct VkInstance_T;
struct VkPhysicalDevice_T;

namespace Cinnamon {
	namespace GraphicsContext 
	{
		Errr Initialize();
		void Shutdown();

		VkInstance_T*			GetInstance();
		VkPhysicalDevice_T*		GetPhysicalDevice();
		VkAllocationCallbacks*	GetAllocator();
	}
}