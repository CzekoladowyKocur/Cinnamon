#include "CinnamonEditor/include/EditorLayer.hpp"
#include "CinnamonEditor/include/EditorSettings.hpp"
#include "CinnamonEditor/include/Project.hpp"
/* Core */
#include "Cinnamon/include/Core/Logger.hpp"
#include "Cinnamon/include/Core/Input.hpp"
#include "Cinnamon/include/Core/Application.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Cinnamon/include/Event/KeyEvent.hpp"
/* Rendering */
#include "Cinnamon/include/Renderer/Renderer.hpp"
/* Scene */
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"
#include "Cinnamon/include/Scene/SceneSerializer.hpp"
/* Panels */
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"
#include "CinnamonEditor/include/Panels/EditorViewportPanel.hpp"
#include "CinnamonEditor/include/Panels/SceneHierarchyPanel.hpp"
#include "CinnamonEditor/include/Panels/ContentBrowserPanel.hpp"
#include "CinnamonEditor/include/Panels/EntityPropertiesPanel.hpp"
/* GUI */
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

InternalScope void BeginDockspace();
InternalScope void EndDockspace();

using namespace Cinnamon;
EditorLayer::EditorLayer(
	EditorSettings& editorSettings,
	const STL::Unique<Window>& window,
	const STL::Unique<Renderer>& renderer,
	const STL::Unique<AssetManager>& assetManager) noexcept
	:
	m_EditorSettings(editorSettings),
	m_Project(nullptr),
	m_Window(window),
	m_Renderer(renderer),
	m_AssetManager(assetManager),
	m_SceneContext(nullptr),
	m_Panels(),
	m_OpenNewProjectPopup(false),
	m_OpenNewScenePopup(false)
{
	const STL::Filepath& lastProjectPath{ m_EditorSettings.LastProjectPath };
	if (not lastProjectPath.empty() and std::filesystem::exists(lastProjectPath))
	{
		m_Project = cinew Project();
		{
			try
			{
				ProjectSerializer projectSerializer(m_Project);
				projectSerializer.Deserialize(lastProjectPath);
			}
			catch ([[maybe_unused]] const std::exception& error)
			{
				CIN_WARN("Failed deserializing last project: {}", error.what());
				return;
			}

			const STL::Filepath& startScenePath{ m_Project->GetStartScenePath() };
			if (not startScenePath.empty() and std::filesystem::exists(startScenePath))
			{
				m_SceneContext = cinew Scene();
				{
					if (not (SceneSerializer(m_SceneContext, m_AssetManager) << startScenePath))
						CIN_WARN("Failed loading start scene {}", startScenePath.string());
					else
					{
						m_CurrentScenePath = startScenePath;
						UpdateWindowTitle();
					}
				}
			}
		}
	}

	if (not m_SceneContext)
		EmptyScene();
}

EditorLayer::~EditorLayer() noexcept
{
	[[likely]]
	if (m_SceneContext)
		cindel m_SceneContext;

	m_SceneContext = nullptr;

	[[likely]]
	if (m_Project)
	{
		m_EditorSettings.LastProjectPath = m_Project->GetProjectPath();
		cindel m_Project;
	}

	m_Project = nullptr;
}

void EditorLayer::OnAttach()
{
	CIN_TRACE("Attaching editor layer");

	const auto [windowWidth, windowHeight]{ m_Window->GetSize() };
	m_Panels.emplace_back(cinew EditorViewportPanel(m_Project, m_SceneContext, m_SelectionContext, m_Renderer, m_AssetManager, windowWidth, windowHeight));
	m_Panels.emplace_back(cinew SceneHierarchyPanel(m_Project, m_SceneContext, m_SelectionContext));
	m_Panels.emplace_back(cinew ContentBrowserPanel(m_Project, m_SceneContext, m_SelectionContext));
	m_Panels.emplace_back(cinew EntityPropertiesPanel(m_Project, m_SceneContext, m_SelectionContext, m_AssetManager));
}

void EditorLayer::OnUpdate(const Timestep timestep)
{
	for (EditorPanelBase* panel : m_Panels)
		panel->OnUpdate(timestep);
	
	ImGui::ShowDemoWindow();
	BeginDockspace();
	
	if (ImGui::BeginMenuBar())
	{
		[[unlikely]]
		if (ImGui::BeginMenu("File"))
		{
			/* Project files */
			[[unlikely]]
			if (ImGui::MenuItem("New Project"))
				NewProject();

			[[unlikely]]
			if (ImGui::MenuItem("Open Project"))
				OpenProject();

			[[unlikely]]
			if (ImGui::MenuItem("Save Project"))
			{
				CIN_INFO("Saving project");
			}

			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
			/* scene */
			[[unlikely]]
			if (ImGui::MenuItem("New Scene", "Ctrl + N"))
				NewScene();

			[[unlikely]]
			if (ImGui::MenuItem("Open Scene", "Ctrl + O"))
				OpenScene();

			ImGui::BeginDisabled(m_SceneContext == nullptr);
			[[unlikely]]
			if (ImGui::MenuItem("Save scene", "Ctrl + S"))
				SaveScene();

			[[unlikely]]
			if (ImGui::MenuItem("Save scene as", "Ctrl + Shift + S"))
				SaveSceneAs();

			ImGui::EndDisabled();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
			[[unlikely]]
			if (ImGui::MenuItem("Quit"))
			{
				CIN_INFO("Quitting. . .");
				Cinnamon::Application::Close();
			}

			ImGui::EndMenu();
		}

		ImGui::BeginDisabled(not m_Project);
		[[unlikely]]
		if (ImGui::BeginMenu("Project"))
		{
			[[unlikely]]
			if (ImGui::MenuItem("Select start scene"))
			{
				if (const STL::Optional<STL::Filepath> startScenePath{ Platform::SelectFile(s_CinnamonSceneExtensionFilter) })
				{
					try
					{
						m_Project->SetStartScenePath(m_CurrentScenePath);
						ProjectSerializer projectSerializer(m_Project);
						projectSerializer.Serialize(m_Project->GetProjectPath());
					}
					catch (const std::exception& error)
					{
						CIN_ERROR("Failed selecting start scene: {}", error.what());
					}
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndDisabled();

		ImGui::EndMenuBar();
	}
	
	for (EditorPanelBase* panel : m_Panels)
		panel->OnGUIRender();

	EndDockspace();
	NewProjectPopup();
	NewScenePopup();
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

	for (EditorPanelBase* const panel : m_Panels)
		panel->OnEvent(event);
}

bool EditorLayer::OnKeyPressed(const KeyPressedEvent& event)
{
	/* Scene keybinds */
	if (Input::IsKeyPressed(Key::Control))
	{
		switch (event.GetKey())
		{
			/* New scene */
			case Key::N:
			{
				NewScene();
			} break;

			/* Open scene */
			case Key::O:
			{
				OpenScene();
			} break;

			/* Save scene */
			case Key::S:
			{
				if (Input::IsKeyPressed(Key::Shift))
					SaveSceneAs();
				else
					SaveScene();
			} break;

			default:
				break;
		}
	}
	
	return false;
}

void EditorLayer::NewProject()
{
	m_OpenNewProjectPopup = true;
}

void EditorLayer::OpenProject()
{
	const STL::Optional<STL::Filepath> openedFile{ Platform::SelectFile(s_CinnamonProjectExtensionFilter)};
	if (openedFile.has_value())
	{
		try
		{
			if (m_Project)
				cindel m_Project;

			m_Project = cinew Project();
			ProjectSerializer projectSerializer(m_Project);
			projectSerializer.Deserialize(openedFile.value());
		}
		catch (const std::exception& error)
		{
			CIN_ERROR("Failed to open project with path {}: {}", openedFile.value().string(), error.what());
		}
	}
}

void EditorLayer::SaveProject()
{}

void EditorLayer::NewScene()
{
	m_SelectionContext = Entity();
	if (not m_SceneContext)
	{
		EmptyScene();
		return;
	}
	
	m_OpenNewScenePopup = true;
}

void EditorLayer::OpenScene()
{	
	m_SelectionContext = Entity();
	if (const STL::Optional<STL::Filepath> filepath{ Platform::SelectFile(s_CinnamonSceneExtensionFilter) })
	{
		if (m_SceneContext)
			cindel m_SceneContext;
		
		m_SceneContext = cinew Scene();
		if (not (SceneSerializer(m_SceneContext, m_AssetManager) << *filepath))
			CIN_ERROR("Failed saving scene with path {}", filepath.value().string());
		else
		{
			m_CurrentScenePath = *filepath;
			UpdateWindowTitle();
		}
	}
}

void EditorLayer::SaveScene()
{
	if (m_SceneContext)
	{
		if (m_CurrentScenePath.empty())
		{
			SaveSceneAs();
			return;
		}

		CIN_ASSERT(std::filesystem::exists(m_CurrentScenePath));
		if(not (SceneSerializer(m_SceneContext, m_AssetManager) >> m_CurrentScenePath))
			CIN_ERROR("Failed to save current scene to {}", m_CurrentScenePath.string());
	}
	else
		CIN_INFO("Attempted saving an empty scene!");
}

void EditorLayer::SaveSceneAs()
{
	m_SelectionContext = Entity();
	if (m_SceneContext)
	{
		if (const STL::Optional<STL::Filepath> filepath{ Platform::SaveFileAs(s_CinnamonSceneExtensionFilter) })
		{
			if (not (SceneSerializer(m_SceneContext, m_AssetManager) >> *filepath))
				CIN_ERROR("Failed to serialize scene");
			else
				m_CurrentScenePath = *filepath;
		}
	}
	else
		CIN_INFO("Attempted saving an empty scene!");
}

void EditorLayer::NewProjectPopup()
{
	if (m_OpenNewProjectPopup)
	{
		m_OpenNewProjectPopup = false;
		ImGui::OpenPopup("New project##NewProjectPopup");
	}

	if (ImGui::IsPopupOpen("New project##NewProjectPopup", 0U))
	{
		ImGui::SetNextWindowSizeConstraints(
			ImVec2{ 500.0f, 180.0f },
			ImVec2{ -1.0f, -1.0f });

		ImGui::SetNextWindowPos(
			ImGui::GetMainViewport()->GetCenter(),
			ImGuiCond_Always,
			ImVec2{ 0.5f, 0.5f });

		constexpr ImGuiWindowFlags popupFlags{ ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavInputs };
		if (ImGui::BeginPopupModal("New project##NewProjectPopup", nullptr, popupFlags))
		{
			ImGui::Text("Create new project\n");
			ImGui::Separator();

			constexpr size_t g_ProjectNameBufferSize{ 256U };
			static char f_ProjectNameBuffer[g_ProjectNameBufferSize]{ '\0' };

			constexpr size_t g_ProjectDirectoryBufferSize{ 256U };
			static char f_ProjectDirectoryBuffer[g_ProjectDirectoryBufferSize]{ '\0' };
			{
				ImGui::SetNextItemWidth(400.0f);

				ImGui::InputTextWithHint(
					"##inputProjectName",
					"Name",
					f_ProjectNameBuffer,
					g_ProjectNameBufferSize);
			}

			{
				static bool f_DirectoryExists{ true };
				ImGui::SetNextItemWidth(400.0f);

				bool showProjectPathAsInvalid{ !f_DirectoryExists };
				if (showProjectPathAsInvalid)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f });

				if (ImGui::InputTextWithHint(
					"##inputProjectDirectory",
					"Location",
					f_ProjectDirectoryBuffer,
					g_ProjectDirectoryBufferSize))
				{
					if (strlen(f_ProjectDirectoryBuffer))
						f_DirectoryExists = FileExists(f_ProjectDirectoryBuffer);
					else
						f_DirectoryExists = true;
				}

				if (showProjectPathAsInvalid)
					ImGui::PopStyleColor();

				ImGui::SameLine();
				if (ImGui::Button("Browse"))
				{
					const std::optional<STL::Filepath> directoryPath{ Platform::SelectDirectory() };

					if (directoryPath.has_value())
					{
						const STL::String directoryPathString{ directoryPath.value().string() };

						if (directoryPathString.size() >= g_ProjectDirectoryBufferSize)
							CIN_ERROR("Project directory is too long");
						else
						{
							memcpy(f_ProjectDirectoryBuffer, directoryPathString.data(), directoryPathString.size());
							f_DirectoryExists = true;
						}
					}
				}
			}

			if (ImGui::Button("Create", ImVec2{ 120.0f, 0.0f }))
			{
				const STL::String projectName{ f_ProjectNameBuffer };
				if (projectName.empty())
				{
					CIN_ERROR("Project name must be specified");
					ImGui::EndPopup();
					return;
				}

				if (std::filesystem::path(projectName).has_extension())
				{
					CIN_ERROR("Project name must not have an extension");
					ImGui::EndPopup();
					return;
				}

				const STL::Filepath projectDirectory{ f_ProjectDirectoryBuffer };
				if (projectDirectory.empty())
				{
					CIN_ERROR("Project location must be specified");
					ImGui::EndPopup();
					return;
				}

				if (!FileExists(f_ProjectDirectoryBuffer))
				{
					CIN_ERROR("Directory {} doesn't exist!", f_ProjectDirectoryBuffer);
					ImGui::EndPopup();
					return;
				}

				try
				{
					const STL::Filepath newProjectWorkDirectory{ projectDirectory / projectName };
					const STL::Filepath newProjectPath{ newProjectWorkDirectory / (projectName + s_CinnamonProjectExtension) };

					if (std::filesystem::exists(newProjectWorkDirectory))
					{
						CIN_ERROR("Failed creating project: directory {} already exists!", newProjectWorkDirectory.string());
						ImGui::EndPopup();
						return;
					}

					std::filesystem::create_directory(newProjectWorkDirectory);
					const ProjectSettings projectSettings
					{ 
						.Directory{ newProjectWorkDirectory },
						.Path{ newProjectPath }
					};

					const STL::Unique<Project> project{ STL::MakeUnique<Project>(projectSettings) };
					ProjectSerializer projectSerializer(project.get());
					projectSerializer.Serialize(newProjectWorkDirectory / (projectName + ".cinproj"));
				}
				catch (const std::exception& error)
				{
					CIN_ERROR("Error: {}", error.what());
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2{ 120.0f, 0.0f }))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}
}

void EditorLayer::NewScenePopup()
{
	if (m_OpenNewScenePopup)
	{
		m_OpenNewScenePopup = false;
		ImGui::OpenPopup("New scene##NewScenePopup");
	}

	if (ImGui::IsPopupOpen("New scene##NewScenePopup", 0U))
	{
		ImGui::SetNextWindowSizeConstraints(
			ImVec2{ 250.0f, 100.0f },
			ImVec2{ -1.0f, -1.0f });

		ImGui::SetNextWindowPos(
			ImGui::GetMainViewport()->GetCenter(),
			ImGuiCond_Always,
			ImVec2{ 0.5f, 0.5f });

		constexpr ImGuiWindowFlags popupFlags{ ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavInputs };
		if (ImGui::BeginPopupModal("New scene##NewScenePopup", nullptr, popupFlags))
		{
			ImGui::TextUnformatted("Any unsaved work will be lost. Proceed?");

			if (ImGui::Button("Yes"))
			{
				EmptyScene();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			
			ImGui::EndPopup();
		}
	}
}

void EditorLayer::EmptyScene()
{
	if (m_SceneContext)
		cindel m_SceneContext;

	m_SelectionContext = Entity();
	m_SceneContext = cinew Scene();
	m_CurrentScenePath.clear();
	UpdateWindowTitle();
}

void EditorLayer::UpdateWindowTitle()
{
	if (not m_CurrentScenePath.empty())
	{
		if (m_CurrentScenePath.has_stem())
		{
			const STL::String sceneName{ m_CurrentScenePath.stem().string() };
			const STL::String windowName{ "Cinnamon Editor - " + sceneName };
			
			m_Window->SetName(windowName.data());
		}
	}
	else	
		m_Window->SetName("Cinnamon Editor");
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