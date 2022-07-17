-- Main project setup file --
WorkspaceName = "Cinnamon"
OutputDirectory = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
CoreProjectName = "Cinnamon"
RendererProjectName = "CinnamonRender"
ScriptingEngineProjectName = "CinnamonScript"
AudioEngineProjectName = "CinnamonAudio"
PhysicsEngineProjectName = "CinnamonPhysics"

workspace (WorkspaceName)
	architecture "x64"
	targetdir (OutputDirectory)

	configurations 
	{ 
		"Debug", 
		"Release",
		"Distribution",
	}

	flags
	{
		"MultiProcessorCompile",
	}

project (CoreProjectName)
	location (CoreProjectName)
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