#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Filesystem.h"

namespace Cinnamon {
	FileWatcher::FileWatcher(const Filepath& path, const STL::InitializerList<STL::String>& observedExtensions, FileWatcherCallback callback) noexcept
		:
		m_IsWatching(true),
		m_ObservedPath(path),
		m_Callback(callback),
		m_ObservedExtensions(observedExtensions),
		m_WatchThread{},
		m_CallbackThread{},
		m_IsSetup{},
		m_CallbackMutex{},
		m_NewCallbackInformation{},
		m_CallbackInformation{}
	{}

	FileWatcher::~FileWatcher() noexcept
	{

	}

	void FileWatcher::WatcherThreadWork()
	{

	}

	void FileWatcher::CallbackThreadWork()
	{
			
	}
}
#endif