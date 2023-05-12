#include "CinnamonEditor/include/Panels/EditorViewportPanel.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Scene/SceneRenderer.hpp"
#include "Cinnamon/include/GUI/GUI.hpp"
#include "Cinnamon/include/Event/WindowEvent.hpp"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

using namespace Cinnamon;

EditorViewportPanel::EditorViewportPanel(
	const STL::Unique<Renderer>& renderer, 
	const uint32_t viewportWidth,
	const uint32_t viewportHeight) noexcept
	:
	m_Renderer(renderer),
	m_SceneRenderer(STL::MakeUnique<SceneRenderer>(renderer, viewportWidth, viewportHeight))
{
	CIN_TRACE("Constructed editor viewport panel");
}

EditorViewportPanel::~EditorViewportPanel() noexcept
{}

void EditorViewportPanel::OnUpdate(const Timestep timestep)
{
	if (m_Renderer)
	{
		m_SceneRenderer->BeginFrame();
		m_SceneRenderer->EndFrame();
	}

	CIN_UNUSED(timestep);
}

void EditorViewportPanel::OnGUIRender()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, { 0.0f });
	ImGui::Begin(GetPanelName());
	ImGui::PopStyleVar(2);
	ImGui::Text("FPS: %f\n", ImGui::GetIO().Framerate);

	const ImVec2 viewportPanelSize{ ImGui::GetContentRegionAvail() };
	GUI::Image(
		m_Renderer, 
		reinterpret_cast<ImageViewID>(m_SceneRenderer->GetFramebuffer()->GetColorAttachmentView()), 
		viewportPanelSize.x, 
		viewportPanelSize.y);

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

void EditorViewportPanel::OnEvent(const Event& event)
{
	switch (event.GetEventType())
	{
		case EEventType::WindowResized:
		{
			const WindowResizedEvent& windowResizedEvent{ static_cast<const WindowResizedEvent&>(event) };
			const auto [windowWidth, windowHeight] { windowResizedEvent.GetResize()};

			m_SceneRenderer->SetViewportSize(windowWidth, windowHeight);			
		} break;

		default: break;
	}

	CIN_UNUSED(event);
}

constexpr const char* EditorViewportPanel::GetPanelName() const
{
	return "Editor Viewport Panel";
}