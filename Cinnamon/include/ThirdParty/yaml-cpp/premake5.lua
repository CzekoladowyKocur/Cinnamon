project "yaml-cpp"
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
		"src/*.cpp",
		"include/*.h"
	}
	
	defines
	{
		"YAML_CPP_STATIC_DEFINE",
	}