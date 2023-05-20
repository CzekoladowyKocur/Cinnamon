project "imguizmo"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. OutputDirectory .. "/%{prj.name}")
	objdir ("bin-int/" .. OutputDirectory .. "/%{prj.name}")

	warnings "Off"

	includedirs
	{
		"../imgui/",
	}

	files
	{
		"GraphEditor.h",
		"GraphEditor.cpp",
		"ImCurveEdit.h",
		"ImCurveEdit.cpp",
		"ImGradient.h",
		"ImGradient.cpp",
		"ImGuizmo.h",
		"ImGuizmo.cpp",
		"ImSequencer.h",
		"ImSequencer.cpp",
		"ImZoomSlider.h",
	}