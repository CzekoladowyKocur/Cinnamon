project "VulkanMemoryAllocator"
	location "."
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")
	warnings "off"

	includedirs 
	{
		VulkanSDKInclude,
	}

	files
	{
		"%{prj.location}/include/*.h",
		"%{prj.location}/source/*.cpp",
	}