project "spirv_cross"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	exceptionhandling "On"

	defines { "SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS" }
	
	files
	{
		"*.cpp",
		"*.c",
		"*.hpp",
		"*.h",
	}

	warnings "Off"