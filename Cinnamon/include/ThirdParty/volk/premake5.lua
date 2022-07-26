project "volk"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	includedirs (VulkanSDKInclude)

	files
	{
		"volk.h",
		"volk.c",
	}