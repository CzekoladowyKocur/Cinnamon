project "spirv_cross"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	files
	{
		"*.cpp",
		"*.c",
		"*.hpp",
		"*.h",
	}

	warnings "Off"