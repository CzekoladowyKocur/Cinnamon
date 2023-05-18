#include "CinnamonRuntime/include/RuntimeApplication.hpp"
#include "CinnamonRuntime/include/RuntimeLayer.hpp"
#include "Cinnamon/include/Event/ApplicationEvent.hpp"
#include "Cinnamon/include/Core/LayerStack.hpp"

using namespace Cinnamon;
RuntimeApplication::RuntimeApplication() noexcept
	:
	Application("Runtime", 400U, 400U, false),
	m_RuntimeLayer(cinew RuntimeLayer(m_MainWindow))
{
	PushLayer(m_RuntimeLayer);
}

RuntimeApplication::~RuntimeApplication() noexcept
{
	PopLayer(m_RuntimeLayer);

	[[likely]]
	if (m_RuntimeLayer)
		cindel m_RuntimeLayer;
}

Errr RuntimeApplication::OnUserInitialize()
{
	return Error::Success;
}

void RuntimeApplication::OnUserShutdown()
{}

void RuntimeApplication::OnEvent(const Event& event)
{
	const EventDispatcher dispatcher(event);
	dispatcher.Dispatch<ApplicationRenderEvent>(std::bind(&RuntimeApplication::OnApplicationRender, this, std::placeholders::_1));

	for (Layer* const layer : *m_LayerStack)
		[[likely]] if (not event.IsHandled)
		layer->OnEvent(event);
		else
			break;
}

bool RuntimeApplication::OnApplicationRender(const ApplicationRenderEvent& event)
{
	[[likely]]
	if (not m_Minimized)
	{
		FunctionVariable double f_LastFrameTime{ Platform::GetAbsoluteTime() };
		const double currentTime{ Platform::GetAbsoluteTime() };
		const Timestep timestep{ static_cast<Timestep::Type>(currentTime - f_LastFrameTime) };
		f_LastFrameTime = currentTime;

		m_RuntimeLayer->OnUpdate(timestep);
	}

	CIN_UNUSED(event);
	return true;
}

REGISTER_CINNAMON_APPLICATION(RuntimeApplication)
#include "Cinnamon/include/Core/EntryPoint.hpp"