project "stb_image"
	kind "StaticLib"
	language "C"
	location "."

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	warnings "Off"

	includedirs
	{
		"%{prj.location}/include",
	}

	files
	{
		"include/stb_image/stb_image.h",
		"src/stb_image.c",
	}