#include "Sandbox/include/SandboxLayer.h"
#include "Cinnamon/include/Core/Logger.h"
#include "Cinnamon/include/Event/WindowEvent.h"

using namespace Cinnamon;
SandboxLayer::SandboxLayer() noexcept
{
	CIN_WARN("Spawning 4 windows. . .");

	for (size_t i{ 0U }; i < 4U; ++i)
	{
		const STL::String windowName{ STL::String("Window ") + std::to_string(i) };
		m_Windows.push_back(STL::MakeUnique<Window>(WindowProperties{ windowName.c_str(), 400, 200, EWindowMode::Windowed, false }));
		m_Renderers.push_back(STL::MakeUnique<Renderer>(m_Windows[i]));
		m_Renderers.back()->SetClearColor(1.0f / float(i + 1U), 0.3f, 0.3f, 1.0f);
	}
}

SandboxLayer::~SandboxLayer() noexcept
{}

void SandboxLayer::OnAttach()
{}

void SandboxLayer::OnUpdate(const Timestep /*timestep*/)
{
	for (size_t i{ 0U }; i < m_Renderers.size(); ++i)
	{
		m_Renderers[i]->BeginFrame();
		m_Renderers[i]->EndFrame();
	}
}

void SandboxLayer::OnDetach()
{}

void SandboxLayer::OnEvent(const Event& /*event*/)
{}