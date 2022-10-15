#include "CinnamonEditor/include/Panels/SceneHierarchyPanel.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

void SceneHierarchyPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void SceneHierarchyPanel::OnGUIRender()
{
	const auto& colors{ ImGui::GetStyle().Colors };
	ImGui::PushStyleColor(ImGuiCol_WindowBg, colors[ImGuiCol_FrameBg]);
	
	ImGui::Begin(GetPanelName());
	ImGui::End();

	ImGui::PopStyleColor();
}

void SceneHierarchyPanel::OnEvent(const Cinnamon::Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* SceneHierarchyPanel::GetPanelName() const
{
	return "Scene Hierarchy Panel";
}
