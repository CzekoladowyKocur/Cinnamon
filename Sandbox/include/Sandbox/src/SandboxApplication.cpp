#include "Sandbox/include/SandboxApplication.h"

SandboxApplication::SandboxApplication() noexcept
	:
	Application()
{}

bool SandboxApplication::OnUserInitialize()
{
	return true;
}

bool SandboxApplication::OnUserShutdown()
{
	return true;
}

REGISTER_CINNAMON_APPLICATION(SandboxApplication);
#include "Cinnamon/include/Core/EntryPoint.h"
