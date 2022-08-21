-- Main project setup file --
WorkspaceName = "Cinnamon"
OutputDirectory = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
CoreProjectName = "Cinnamon"
EditorProjectName = "CinnamonEditor"
SandboxProjectName = "Sandbox"

VulkanSDK = os.getenv("VULKAN_SDK");
VulkanLibrary = VulkanSDK .. "/Lib/vulkan-1.lib"
VulkanSDKInclude = VulkanSDK .. "/Include"
FMTInclude = "Cinnamon/include/ThirdParty/fmt/include"

-- Use volk as a static lib from SDK? (Can't use debug symbols)
-- VolkInclude = VulkanSDK;
-- VolkLibrary = VulkanSDK .. "/Lib/volk.lib"

workspace (WorkspaceName)
	architecture "x64"
	platforms "x64"
	targetdir (OutputDirectory)
	startproject (EditorProjectName) 

	configurations 
	{ 
		"Debug", 
		"Release",
		"Distribution",
	}

	flags 
	{
		"MultiProcessorCompile",
		"FatalCompileWarnings", -- Treat compiler warnings as errors
		"FatalLinkWarnings", -- Treat linker warnings as errors,
	}

	warnings "Extra" -- Warning level
	exceptionhandling "Off" -- Disable exception handling

	filter "configurations:Debug"
		defines "CIN_DEBUG" -- Engine define
		symbols "On" -- Specifies whether the compiler should generate debug symbols
		optimize "Off" -- Disables Optimization
		-- optimize "Debug" -- Optimization with some debugger step-through support
		runtime "Debug" -- Specifies the type of runtime library to use
		staticruntime "on" -- Sets runtime library to "MultiThreaded" instead of "MultiThreadedDLL"

	filter "configurations:Release"
		defines "CIN_RELEASE"
		defines "NDEBUG" -- Explicitly define NDEBUG
		symbols "On" 
		optimize "On" 
		runtime "Release"
		staticruntime "on"

	filter "configurations:Distribution"
		defines "CIN_DISTRIBUTION"
		defines "NDEBUG" -- Explicitly define NDEBUG
		symbols "Off" 
		optimize "Speed" -- Optimize for speed
		-- optimize "Full" -- Perform full optimization 
		runtime "Release"
		staticruntime "on"
		flags "LinkTimeOptimization" -- Enable link time optimization

		-- Platforms --
	filter "system:windows"
		defines "CIN_PLATFORM_WINDOWS"
		defines "VK_USE_PLATFORM_WIN32_KHR"
		systemversion "latest"
		entrypoint "wWinMainCRTStartup"

	filter "system:linux"
		defines "CIN_PLATFORM_LINUX"
        defines "VK_USE_PLATFORM_WAYLAND_KHR"

	filter "system:macosx"
		defines "CIN_PLATFORM_MACOS"

	filter "system:ios"
		defines "CIN_PLATFORM_IOS"

	filter "system:android"
		defines "CIN_PLATFORM_ANDROID"

group "ThirdParty"
include "Cinnamon/include/ThirdParty/xdg"
include "Cinnamon/include/ThirdParty/volk"
include "Cinnamon/include/ThirdParty/fmt"
group ""

project (CoreProjectName)
	location (CoreProjectName .. "/include")
	language "C++"
	cppdialect "C++20"
	kind "StaticLib"

	targetdir ("bin/" .. (OutputDirectory) .. "/%{prj.name}")
	objdir ("bin-int/" .. (OutputDirectory) .. "/%{prj.name}")

	files 
	{ 
		"%{prj.name}/include/Cinnamon/**.h", 
		"%{prj.name}/include/Cinnamon/**.hpp", 

		"%{prj.name}/include/Cinnamon/**.c", 
		"%{prj.name}/include/Cinnamon/**.cpp",
		-------------------------------------
		"%{prj.name}/include/Platform/**.h", 
		"%{prj.name}/include/Platform/**.hpp", 
							 
		"%{prj.name}/include/Platform/**.c", 
		"%{prj.name}/include/Platform/**.cpp",
		-------------------------------------
	}

	includedirs
	{
		"%{prj.name}/include",
		VulkanSDKInclude,
		FMTInclude,
	}
	
	links
	{
		"volk",
	}

	filter "system:linux"
		links
        {
			"wayland-client",
			"xdg"
        }

project (EditorProjectName)
	location (EditorProjectName .. "/include")
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"

	targetdir ("bin/" .. (OutputDirectory) .. "/%{prj.name}")
	objdir ("bin-int/" .. (OutputDirectory) .. "/%{prj.name}")

	files 
	{ 
		"%{prj.name}/include/CinnamonEditor/**.h", 
		"%{prj.name}/include/CinnamonEditor/**.hpp", 

		"%{prj.name}/include/CinnamonEditor/**.c", 
		"%{prj.name}/include/CinnamonEditor/**.cpp",
		-------------------------------------
	}

	includedirs
	{
		"%{prj.name}/include",
		"%{wks.location}/Cinnamon/include",
		FMTInclude,
	}

	links "Cinnamon"

project (SandboxProjectName)
	location (SandboxProjectName .. "/include")
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"

	targetdir ("bin/" .. (OutputDirectory) .. "/%{prj.name}")
	objdir ("bin-int/" .. (OutputDirectory) .. "/%{prj.name}")

	files 
	{ 
		"%{prj.name}/include/Sandbox/**.h", 
		"%{prj.name}/include/Sandbox/**.hpp", 
							 
		"%{prj.name}/include/Sandbox/**.c", 
		"%{prj.name}/include/Sandbox/**.cpp",
		-------------------------------------
	}

	includedirs
	{
		"%{prj.name}/include",
		"%{wks.location}/Cinnamon/include",
		FMTInclude,
	}

	links "Cinnamon"