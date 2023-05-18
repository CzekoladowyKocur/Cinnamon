#include "CinnamonEditor/include/EditorApplication.hpp"
#include "CinnamonEditor/include/EditorLayer.hpp"
#include "CinnamonEditor/include/EditorSettings.hpp"

#include "Cinnamon/include/Core/LayerStack.hpp"
#include "Cinnamon/include/Core/Logger.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Cinnamon/include/Core/Layer.hpp"
#include "Cinnamon/include/Event/WindowEvent.hpp"
#include "Cinnamon/include/Event/ApplicationEvent.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Asset/AssetManager.hpp"
#include "Cinnamon/include/GUI/GUIRenderer.hpp"
#include <stdexcept>

InternalScope constexpr const char* s_LocalEditorSettingsFilePath{ "EditorSettings.ini" };

using namespace Cinnamon;
EditorApplication::EditorApplication() noexcept
	:
	Application("Cinnamon Editor", 1280U, 960U, false),
	m_Settings(LoadLocalEditorSettings()),
	m_Renderer(STL::MakeUnique<Renderer>(m_MainWindow)),
	m_GUIRenderer(STL::MakeUnique<GUIRenderer>(m_MainWindow, m_Renderer)),
	m_AssetManager(STL::MakeUnique<AssetManager>(m_Renderer->GetAllocator())),
	m_EditorLayer(cinew EditorLayer(m_Settings, m_MainWindow, m_Renderer, m_AssetManager))
{}

EditorApplication::~EditorApplication() noexcept
{
	CIN_ASSERT(m_EditorLayer);
	cindel m_EditorLayer;

	SaveLocalEditorSettings(m_Settings);
}

Errr EditorApplication::OnUserInitialize()
{
	PushLayer(m_EditorLayer);
	CIN_TRACE("Pushed editor application layer");

	return Error::Success;
}

void EditorApplication::OnUserShutdown()
{
	PopLayer(m_EditorLayer);
	CIN_TRACE("Popped editor application layer");
}

void EditorApplication::OnEvent(const Event& event)
{
	const EventDispatcher dispatcher(event);
	dispatcher.Dispatch<ApplicationRenderEvent>(std::bind(&EditorApplication::OnApplicationRender, this, std::placeholders::_1));

	for (Layer* const layer : *m_LayerStack)
		[[likely]] if (not event.IsHandled)
			layer->OnEvent(event);
		else
			break;

	/* Separate update. */
	m_GUIRenderer->OnEvent(event);
}

bool EditorApplication::OnWindowResized(const WindowResizedEvent& event)
{
	const auto [width, height]{ event.GetResize() };

	[[likely]]
	if (not m_Minimized)
		m_Renderer->SetViewportSize(width, height);

	return false;
}

bool EditorApplication::OnApplicationRender(const ApplicationRenderEvent& event)
{
	[[likely]]
	if (not m_Minimized)
	{
		FunctionVariable double f_LastFrameTime{ Platform::GetAbsoluteTime() };
		const double currentTime{ Platform::GetAbsoluteTime() };
		const Timestep timestep{ static_cast<Timestep::Type>(currentTime - f_LastFrameTime) };
		f_LastFrameTime = currentTime;

		m_Renderer->BeginFrame();
		{
			m_GUIRenderer->BeginFrame();
			m_EditorLayer->OnUpdate(timestep);
			m_GUIRenderer->EndFrame();
		}
		m_Renderer->EndFrame();
	}
	
	CIN_UNUSED(event);
	return true;
}

EditorSettings EditorApplication::LoadLocalEditorSettings()
{
	EditorSettings settings;

	try
	{
		if (not std::filesystem::exists(s_LocalEditorSettingsFilePath))
			throw std::runtime_error("Local editor settings file doesn't exist");

		std::ifstream editorSettingsFile(s_LocalEditorSettingsFilePath, std::ios::binary);
		if (!editorSettingsFile.is_open())
			throw std::runtime_error("Failed opening local editor settings file");

		editorSettingsFile >> settings.LastProjectPath;
	}
	catch ([[maybe_unused]] const std::exception& error)
	{
		CIN_WARN("Failed loading local editor settings: {}. Using default settings.", error.what());
	}

	return settings;
}

void EditorApplication::SaveLocalEditorSettings(const EditorSettings& editorSettings)
{
	try
	{
		std::ofstream editorSettingsFile(s_LocalEditorSettingsFilePath, std::ios::binary);
		if (!editorSettingsFile.is_open())
			throw std::runtime_error("Failed opening local editor settings file");

		editorSettingsFile << editorSettings.LastProjectPath;
	}
	catch (const std::runtime_error& error)
	{
		CIN_ERROR("Failed saving local editor settings project: {}", error.what());
	}
}

REGISTER_CINNAMON_APPLICATION(EditorApplication);
#include "Cinnamon/include/Core/EntryPoint.hpp"