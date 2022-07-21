-- Main project setup file --
WorkspaceName = "Cinnamon"
OutputDirectory = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
CoreProjectName = "Cinnamon"
EditorProjectName = "CinnamonEditor"

workspace (WorkspaceName)
	architecture "x64"
	platforms "x64"
	targetdir (OutputDirectory)

	startproject (Cinnamon) -- TODO: REMOVE ASAP

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

	filter "system:macosx"
		defines "CIN_PLATFORM_MACOS"

	filter "system:ios"
		defines "CIN_PLATFORM_IOS"

	filter "system:android"
		defines "CIN_PLATFORM_ANDROID"

local xdg = "%{wks.location}" .. "Cinnamon/include/ThirdParty/xdg"
group "ThirdParty"
{
	include (xdg)
}

project (CoreProjectName)
	location (CoreProjectName .. "/include")
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"

	targetdir ("bin/" .. (OutputDirectory) .. "/%{prj.name}")
	objdir ("bin-int/" .. (OutputDirectory) .. "/%{prj.name}")

	files 
	{ 
		"%{prj.name}/include/**.h", 
		"%{prj.name}/include/**.c", 
		"%{prj.name}/include/**.hpp", 
		"%{prj.name}/include/**.cpp", 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp", 
	}

	includedirs
	{
		"%{prj.name}/include",
	}

	filter "system:linux"
		links (xdg)

project (EditorProjectName)
	location (EditorProjectName .. "/include")
	language "C++"
	cppdialect "C++20"
	kind "ConsoleApp"

	targetdir ("bin/" .. (OutputDirectory) .. "/%{prj.name}")
	objdir ("bin-int/" .. (OutputDirectory) .. "/%{prj.name}")

	files 
	{ 
		"%{prj.name}/include/**.h", 
		"%{prj.name}/include/**.h", 
		"%{prj.name}/include/**.c", 
		"%{prj.name}/include/**.hpp", 
		"%{prj.name}/include/**.cpp", 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp", 
	}

	includedirs
	{
		"%{prj.name}/include",
		"%{wks.location}/Cinnamon/include",
	}