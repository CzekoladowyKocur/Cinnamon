#include "Sandbox/include/SandboxApplication.h"
#include "Cinnamon/include/Core/Filesystem.h"

using namespace Cinnamon;

SandboxApplication::SandboxApplication() noexcept
	:
	Application()
{}

bool SandboxApplication::OnUserInitialize()
{
	FileWatcher* fileWatcher{ cinew FileWatcher("/home/dxm/Container", { ".txt", ".cpp" }, [](const STL::String file, const Cinnamon::EFileAction action)
		{
			switch (action)
			{
				case Cinnamon::EFileAction::Created:
				{
					CIN_INFO("Created file: {}", file);
				} break;

				case Cinnamon::EFileAction::Modified:
				{
					CIN_INFO("Modified file: {}", file);
				} break;

				case Cinnamon::EFileAction::Deleted:
				{
					CIN_INFO("Deleted file: {}", file);
				} break;

				default:
					break;
			}
			CIN_UNUSED(file);
	}) };

	cindel fileWatcher;

	CIN_UNUSED(fileWatcher);
	return true;
}

bool SandboxApplication::OnUserShutdown()
{
	return true;
}

REGISTER_CINNAMON_APPLICATION(SandboxApplication);
#include "Cinnamon/include/Core/EntryPoint.h"
