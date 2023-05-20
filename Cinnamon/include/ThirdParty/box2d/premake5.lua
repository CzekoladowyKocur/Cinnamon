project "box2d"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	warnings "Off"

	includedirs
	{
		"include/",
	}

	files
	{
		"src/collision/*.cpp",
		"src/dynamics/*.cpp",
		"src/rope/*.cpp",
		"src/common/*.cpp",
		"include/*.h",
	}