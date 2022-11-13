-- Main project setup file --
OutputDirectory = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
VulkanSDK = os.getenv("VULKAN_SDK");
VulkanLibrary = VulkanSDK .. "/Lib/vulkan-1.lib"
VulkanSDKInclude = VulkanSDK .. "/include"
FMTInclude = "Cinnamon/include/ThirdParty/fmt/include"

-- Use volk as a static lib from SDK? (Can't use debug symbols)
-- VolkInclude = VulkanSDK;
-- VolkLibrary = VulkanSDK .. "/Lib/volk.lib"

workspace ("Cinnamon")
	architecture "x64"
	platforms "x64"
	startproject ("CinnamonEditor") 
	targetdir (OutputDirectory)

	configurations 
	{ 
		"Debug",
		"Debug-Optimized",
		"Release",
		"Distribution",
	}

	flags 
	{
		"MultiProcessorCompile",	-- Multicore compilation
		"FatalCompileWarnings",		-- Treat compiler warnings as errors
		"FatalLinkWarnings",		-- Treat linker warnings as errors,
	}

	warnings "Extra"				-- Warning level
	exceptionhandling "Off"			-- Disable exception handling as default

	filter "configurations:Debug"
		defines						-- Engine defines
		{
			"CIN_DEBUG",
		}
		
		symbols "On"				-- Specifies whether the compiler should generate debug symbols
		optimize "Off"				-- Disables Optimization									
		runtime "Debug"				-- Specifies the type of runtime library to use
		staticruntime "on"			-- Links C/C++ runtime as a static library

	filter "configurations:Debug-Optimized"
		defines
		{
			"CIN_DEBUG",
			"CIN_OPTIMIZED_DEBUG",
		}

		symbols "On"				-- Specifies whether the compiler should generate debug symbols
		optimize "Debug"			-- Optimization with some debugger step-through support
		runtime "Debug"				-- Specifies the type of runtime library to use
		staticruntime "on"			-- Links C/C++ runtime as a static library

	filter "configurations:Release"
		defines						-- Explicitly define NDEBUG
		{
			"CIN_RELEASE",
			"NDEBUG",
		}

		symbols "On" 
		optimize "On" 
		runtime "Release"
		staticruntime "on"

	filter "configurations:Distribution"
		defines						-- Explicitly define NDEBUG
		{
			"CIN_DISTRIBUTION",
			"NDEBUG",
		}

		symbols "Off" 
		optimize "Speed"			-- Optimize for speed
		optimize "Full"				-- Perform full optimization 
		runtime "Release"
		flags "LinkTimeOptimization"-- Enable link time optimization
		staticruntime "on"

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
include "Cinnamon/include/ThirdParty/imgui"
group ""

project ("Cinnamon")
	location ("Cinnamon/include")
	language "C++"
	cppdialect "C++20"
	kind "StaticLib"
	buildmessage "Building engine core"

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
		"imgui",
	}

	filter "system:linux"
		links
        {
			"wayland-client",
			"xdg"
        }

project ("CinnamonEditor")
	location ("CinnamonEditor/include")
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"
	buildmessage "Building engine editor"

	targetdir ("bin/" .. (OutputDirectory) .. "/%{prj.name}")
	objdir ("bin-int/" .. (OutputDirectory) .. "/%{prj.name}")

	exceptionhandling "On"	-- Enable exception handling for editor

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

	links
	{
		"Cinnamon",
		"imgui",
	}

	postbuildcommands 
	{
		"{COPY} Resources %{cfg.targetdir}/Resources",
		"{COPY} imgui.ini %{cfg.targetdir}",
	}

	filter "system:linux"
		links
        {
			"wayland-client",
			"xdg"
        }

	filter "configurations:Distribution"
		defines 
		{
			"_HAS_EXCEPTIONS=0",
		}
		kind "WindowedApp"

project ("Sandbox")
	location ("Sandbox/include")
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

	links
	{
		"Cinnamon",
		"imgui",
	}
		
	postbuildcommands 
	{
		"{COPY} %{wks.location}/CinnamonEditor/include/Resources %{cfg.targetdir}/Resources",
	}

	filter "system:linux"
		links
        {
			"wayland-client",
			"xdg"
        }