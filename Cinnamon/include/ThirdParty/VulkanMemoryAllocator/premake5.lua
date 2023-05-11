project "VulkanMemoryAllocator"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")
	warnings "off"

	includedirs 
	{
		VulkanIncludeDirectory,
		"%{prj.location}/include",
	}

	files
	{
		"%{prj.location}/include/*.h",
		"%{prj.location}/source/*.cpp",
	}