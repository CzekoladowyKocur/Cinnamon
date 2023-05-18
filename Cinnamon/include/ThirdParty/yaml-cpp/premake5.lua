project "yaml-cpp"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	warnings "Off"
	exceptionhandling "On"

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
		"YAML_CPP_NO_EXCEPTIONS"
	}