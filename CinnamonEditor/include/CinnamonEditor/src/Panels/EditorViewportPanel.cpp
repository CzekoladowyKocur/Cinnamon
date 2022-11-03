#include "CinnamonEditor/include/Panels/EditorViewportPanel.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

using namespace Cinnamon;

void EditorViewportPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void EditorViewportPanel::OnGUIRender()
{
	ImGui::Begin(GetPanelName());
	ImGui::Text("FPS: %f\n", ImGui::GetIO().Framerate);

	[[unlikely]]
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload{ ImGui::AcceptDragDropPayload("Script File") })
		{
			CIN_WARN("Dragging a script file. . .");
			CIN_UNUSED(payload);
		}

		ImGui::EndDragDropTarget();
	}

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