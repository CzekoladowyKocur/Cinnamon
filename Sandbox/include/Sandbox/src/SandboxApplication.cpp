#include "Sandbox/include/SandboxApplication.hpp"
#include "Sandbox/include/SandboxLayer.hpp"

#include "Cinnamon/include/Core/LayerStack.hpp"
#include "Cinnamon/include/Event/Event.hpp"
#include "Cinnamon/include/Event/ApplicationEvent.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"

using namespace Cinnamon;
SandboxApplication::SandboxApplication() noexcept
	:
	Application("Sandbox application", 400U, 400U, false),
	m_Renderer(STL::MakeUnique<Renderer>(m_MainWindow)),
	m_SandboxLayer(cinew SandboxLayer(m_MainWindow, m_Renderer))
{
	PushLayer(m_SandboxLayer);
}

SandboxApplication::~SandboxApplication() noexcept
{
	PopLayer(m_SandboxLayer);
	cindel m_SandboxLayer;
}

Errr SandboxApplication::OnUserInitialize()
{
	return Error::Success;
}

void SandboxApplication::OnUserShutdown()
{
}

void SandboxApplication::OnEvent(const Event& event)
{
	const EventDispatcher dispatcher(event);
	dispatcher.Dispatch<ApplicationRenderEvent>(std::bind(&SandboxApplication::OnApplicationRender, this, std::placeholders::_1));

	for (Layer* const layer : *m_LayerStack)
		[[likely]] if (not event.IsHandled)
		layer->OnEvent(event);
	else
		break;
}

bool SandboxApplication::OnApplicationRender(const Cinnamon::ApplicationRenderEvent& event)
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
			m_SandboxLayer->OnUpdate(timestep);
		}
		m_Renderer->EndFrame();
	}

	CIN_UNUSED(event);
	return true;
}

REGISTER_CINNAMON_APPLICATION(SandboxApplication);
#include "Cinnamon/include/Core/EntryPoint.hpp"
