project "xdg"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	filter "system:linux"	
		files
		{
			"xdg-shell.c",
			"xdg-shell.h",
		}
