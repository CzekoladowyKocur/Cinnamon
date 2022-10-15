#include "CinnamonEditor/include/Panels/EditorViewportPanel.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

void EditorViewportPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void EditorViewportPanel::OnGUIRender()
{
	ImGui::Begin(GetPanelName());
	ImGui::End();
}

void EditorViewportPanel::OnEvent(const Cinnamon::Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* EditorViewportPanel::GetPanelName() const
{
	return "Editor Viewport Panel";
}