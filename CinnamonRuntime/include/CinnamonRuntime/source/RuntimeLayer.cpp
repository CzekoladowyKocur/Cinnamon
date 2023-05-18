#include "CinnamonRuntime/include/RuntimeLayer.hpp"
#include "Cinnamon/include/Core/Window.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Scene/SceneRenderer.hpp"
#include "Cinnamon/include/Event/WindowEvent.hpp"
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/SceneSerializer.hpp"
#include "Cinnamon/include/Scene/SceneCamera.hpp"
#include "Cinnamon/include/Asset/AssetManager.hpp"
#include "CinMath/CinMath.h"

using namespace Cinnamon;
RuntimeLayer::RuntimeLayer(const Cinnamon::STL::Unique<Cinnamon::Window>& window) noexcept
	:
	m_Window(window),
	m_Renderer(STL::MakeUnique<Renderer>(window)),
	m_SceneRenderer(STL::MakeUnique<SceneRenderer>(m_Renderer, true, window->GetWidth(), window->GetHeight())),
	m_AssetManager(STL::MakeUnique<AssetManager>(m_Renderer->GetAllocator())),
	m_Scene(STL::MakeUnique<Scene>()),
	m_SceneCamera(STL::MakeUnique<SceneCamera>(static_cast<float>(window->GetWidth()) / window->GetHeight()))
{
	SceneSerializer serializer(m_Scene.get(), m_AssetManager);
	CIN_VERIFY(serializer << "Scenes/IceCubesLight.cinscene");
}

RuntimeLayer::~RuntimeLayer() noexcept
{}

void RuntimeLayer::OnAttach()
{
	m_SceneRenderer->SetRenderedScene(m_Scene.get());
}

void RuntimeLayer::OnUpdate(const Timestep timestep)
{
	m_Renderer->BeginFrame();
	auto proj{ m_SceneCamera->GetViewProjection() };
	proj[5] *= -1.0f;

	m_SceneRenderer->RenderScene(proj);
	m_Renderer->EndFrame();
	CIN_UNUSED(timestep);
}

void RuntimeLayer::OnDetach()
{}

void RuntimeLayer::OnEvent(const Event& event)
{
	switch (event.GetEventType())
	{
		case EEventType::WindowResized:
		{
			const WindowResizedEvent& windowResizedEvent{ static_cast<const WindowResizedEvent&>(event) };
			const auto [width, height] { windowResizedEvent.GetResize()};
			
			if(width > 0U and height > 0U )
			{ 
				m_Renderer->SetViewportSize(width, height);
				m_SceneRenderer->SetViewportSize(width, height);
				m_SceneCamera->SetAspectRatio(static_cast<float>(width) / height);
			}
		}
	}

	CIN_UNUSED(event);
}