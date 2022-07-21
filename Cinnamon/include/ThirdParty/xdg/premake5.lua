project "xdg"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	filter "system:linux"
		systemversion "latest"
		
		files
		{
			"xdg-shell-unstable-v6.c",
			"xdg-shell-unstable-v6.h",
		}