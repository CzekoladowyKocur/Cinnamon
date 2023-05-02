#include "CinnamonEditor/include/EditorLayer.h"
#include "Cinnamon/include/Core/Logger.h"
#include "Cinnamon/include/Core/Input.h"
#include "Cinnamon/include/Core/Application.h"
#include "Cinnamon/include/Event/KeyEvent.h"
#include "Cinnamon/include/Core/Window.h"


/* Panels */
#include "CinnamonEditor/include/Panels/EditorPanelBase.h"
#include "CinnamonEditor/include/Panels/EditorViewportPanel.h"
#include "CinnamonEditor/include/Panels/SceneHierarchyPanel.h"
#include "CinnamonEditor/include/Panels/ContentBrowserPanel.h"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

InternalScope void BeginDockspace();
InternalScope void EndDockspace();

using namespace Cinnamon;
void EditorLayer::OnAttach()
{
	m_Panels.emplace_back(cinew EditorViewportPanel);
	m_Panels.emplace_back(cinew SceneHierarchyPanel);
	m_Panels.emplace_back(cinew ContentBrowserPanel);
}

void EditorLayer::OnUpdate(const Timestep timestep)
{
	for (EditorPanelBase* panel : m_Panels)
		panel->OnUpdate(timestep);
	{
		BeginDockspace();

		[[unlikely]]
		if (ImGui::BeginMenuBar())
		{
			[[unlikely]]
			if (ImGui::BeginMenu("File"))
			{
				/* Project files */
				[[unlikely]]
				if (ImGui::MenuItem("New Project"))
				{
					CIN_INFO("Opening new project");
				}

				[[unlikely]]
				if (ImGui::MenuItem("Open Project"))
				{
					CIN_INFO("Opening project");
				}

				[[unlikely]]
				if (ImGui::MenuItem("Save Project"))
				{
					CIN_INFO("Saving project");
				}

				ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
				[[unlikely]]
				if (ImGui::MenuItem("Quit"))
				{
					CIN_INFO("Quitting. . .");
					Cinnamon::Application::Close();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		
		for (EditorPanelBase* panel : m_Panels)
			panel->OnGUIRender();

		EndDockspace();
	}

#if (CIN_DEBUG && 0)
	FunctionVariable struct {
		uint32_t x{ 0U }, y{ 0U };
	} f_CachedMousePosition;

	const auto [currentMousePositionX, currentMousePositionY] { Cinnamon::Input::GetMousePosition() };
	if (currentMousePositionX != f_CachedMousePosition.x || currentMousePositionY != f_CachedMousePosition.y)
	{
		CIN_TRACE("Mouse moved [new x, new y]: {0}, {1}", currentMousePositionX, currentMousePositionY);
		f_CachedMousePosition.x = currentMousePositionX;
		f_CachedMousePosition.y = currentMousePositionY;
	}

	if (Cinnamon::Input::IsMouseButtonPressed(Cinnamon::Mouse::LeftButton))
		CIN_TRACE("Pressed left mouse button");
	else if (Cinnamon::Input::IsMouseButtonPressed(Cinnamon::Mouse::MiddleButton))
		CIN_TRACE("Pressed middle mouse button");
	else if(Cinnamon::Input::IsMouseButtonPressed(Cinnamon::Mouse::RightButton))
		CIN_TRACE("Pressed right mouse button");

	for (uint32_t i{ 0U }; i < static_cast<uint32_t>(Cinnamon::Key::KeysEnd); ++i)
		if (Cinnamon::Input::IsKeyPressed(static_cast<Cinnamon::Key>(i)))
			CIN_TRACE("Pressed key: {}", Cinnamon::KeyToString(static_cast<Cinnamon::Key>(i)));
	
	//CIN_WARN("Editor layer update timestep: {}", timestep);
	CIN_UNUSED(timestep);
#else
	CIN_UNUSED(timestep);
#endif
}

void EditorLayer::OnDetach()
{
	for (uint32_t i{ 0U }; i < static_cast<uint32_t>(m_Panels.size()); ++i)
	{
		CIN_ASSERT(m_Panels[i], "Invalid panel");
		cindel m_Panels[i];
	}

	m_Panels.clear();
}

void EditorLayer::OnEvent(const Event& event)
{
	const EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>(std::bind(&EditorLayer::OnKeyPressed, this, std::placeholders::_1));
}

bool EditorLayer::OnKeyPressed(const KeyPressedEvent& event)
{
	CIN_UNUSED(event);
	return false;
}

InternalScope void BeginDockspace()
{
	constexpr ImGuiWindowFlags dockSpaceWindowFlags
	{
		ImGuiWindowFlags_MenuBar				|
		ImGuiWindowFlags_NoDocking				|
		ImGuiWindowFlags_NoScrollbar			|
		ImGuiWindowFlags_NoTitleBar				|
		ImGuiWindowFlags_NoCollapse				|
		ImGuiWindowFlags_NoResize				|
		ImGuiWindowFlags_NoMove					|
		ImGuiWindowFlags_NoNavFocus				|
		ImGuiWindowFlags_NoBringToFrontOnFocus	|
		ImGuiWindowFlags_NoBackground 
	};

	/* Set dockspace size to fullscreen */
	ImGuiViewport* const mainViewport{ ImGui::GetMainViewport() };
	ImGui::SetNextWindowPos(mainViewport->Pos);
	ImGui::SetNextWindowSize(mainViewport->Size);
	ImGui::SetNextWindowViewport(mainViewport->ID);

	bool dockspaceOpen{ false };
	constexpr std::string_view dockspaceID{ "Dockspace" };
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::Begin(dockspaceID.data(), &dockspaceOpen, dockSpaceWindowFlags);
	ImGui::PopStyleVar(3);

	constexpr ImGuiDockNodeFlags dockspaceFlags
	{
		//ImGuiDockNodeFlags_HiddenTabBar		|
		//ImGuiDockNodeFlags_NoTabBar			|
		ImGuiDockNodeFlags_NoCloseButton		|
		ImGuiDockNodeFlags_NoWindowMenuButton	|
		ImGuiDockNodeFlags_NoWindowMenuButton 
	};

	const ImGuiID ID{ ImGui::GetID(dockspaceID.data()) };
	if (!ImGui::DockBuilderGetNode(ID))
	{
		ImGui::DockBuilderRemoveNode(ID); // Clear out existing layout
		ImGui::DockBuilderAddNode(ID); // Add empty node
		ImGui::DockBuilderSetNodeSize(ID, { ImGui::GetIO().DisplaySize.x * ImGui::GetIO().DisplayFramebufferScale.x, ImGui::GetIO().DisplaySize.y * ImGui::GetIO().DisplayFramebufferScale.y });
	
		ImGui::DockBuilderFinish(ID);
	}

	const ImGuiID dockID{ ImGui::GetID(dockspaceID.data()) };
	ImGui::DockSpace(dockID, { 0.0f, 0.0f }, dockspaceFlags);
}

InternalScope void EndDockspace()
{
	ImGui::End();
}