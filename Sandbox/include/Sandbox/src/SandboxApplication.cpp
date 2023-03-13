#include "Sandbox/include/SandboxApplication.h"
#include "Cinnamon/include/Core/Filesystem.h"

using namespace Cinnamon;

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
