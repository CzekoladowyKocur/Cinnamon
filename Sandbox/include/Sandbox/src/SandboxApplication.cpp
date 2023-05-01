#include "Sandbox/include/SandboxApplication.h"
#include "Sandbox/include/SandboxLayer.h"

using namespace Cinnamon;
SandboxApplication::SandboxApplication() noexcept
	:
	Application("Sandbox application", 400, 400, false),
	m_SandboxLayer(cinew SandboxLayer)
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

REGISTER_CINNAMON_APPLICATION(SandboxApplication);
#include "Cinnamon/include/Core/EntryPoint.h"
