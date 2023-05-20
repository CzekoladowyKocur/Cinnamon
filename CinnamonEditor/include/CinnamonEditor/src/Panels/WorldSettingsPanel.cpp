#include "CinnamonEditor/include/Panels/WorldSettingsPanel.hpp"
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/Environment.hpp"

#include "Cinnamon/include/GUI/GUI.hpp"
#include "ThirdParty/imgui/imgui.h"

using namespace Cinnamon;
WorldSettingsPanel::WorldSettingsPanel(
	ProjectContext		projectContext,
	SceneContext		sceneContext,
	SelectionContext	selectionContext) noexcept
	:
	EditorPanelBase(projectContext, sceneContext, selectionContext)
{}

void WorldSettingsPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void WorldSettingsPanel::OnGUIRender()
{
	ImGui::Begin(GetPanelName());
	if (m_SceneContext)
	{
		ImGui::ColorEdit3("Ambient Light", m_SceneContext->GetEnvironment()->GetAmbientLightColor());
	}
	ImGui::End();
}

void WorldSettingsPanel::OnEvent(const Cinnamon::Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* WorldSettingsPanel::GetPanelName() const
{
	return "World Settings";
}