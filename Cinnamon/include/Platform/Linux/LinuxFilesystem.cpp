#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Filesystem.h"
#include <sys/inotify.h>

namespace Cinnamon {
	constexpr std::size_t g_BufferSize = { 2 << 16 };
	FileWatcher::FileWatcher(const FilepathT& path, const STL::InitializerList<STL::String>& observedExtensions, FileWatcherCallback callback) noexcept
		:
		m_IsWatching(true),
		m_ObservedPath(path),
		m_Callback(callback),
		m_ObservedExtensions(observedExtensions),
		m_WatchThread{},
		m_IsSetup{},
		m_InotifyHandle(-1),
		m_InotifyWatcherDescriptor(-1)
	{
		m_WatchThread = std::move(std::thread(&FileWatcher::WatcherThreadWork, this));

		std::future<void> future = m_IsSetup.get_future();
		future.get();
	}

	FileWatcher::~FileWatcher() noexcept
	{
		m_IsWatching = false;
		m_IsSetup = std::promise<void>();
		m_WatchThread.join();
	}

	void FileWatcher::WatcherThreadWork()
	{
		CIN_WARN("Obsering path: {}", m_ObservedPath);
		m_InotifyHandle = inotify_init1(IN_NONBLOCK);
		CIN_ASSERT(m_InotifyHandle != -1);
		m_InotifyWatcherDescriptor = inotify_add_watch(m_InotifyHandle, m_ObservedPath.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE);
		CIN_ASSERT(m_InotifyWatcherDescriptor != -1);
		m_IsSetup.set_value();

		STL::Array<std::byte, g_BufferSize> buffer;
		[[likely]]
		while(m_IsWatching)
		{
			const auto length 
			{ 
				read
				(
					m_InotifyHandle,
					buffer.data(),
					buffer.size()
				) 
			};
			
			if(length < 0)
			{
				usleep(50);
				continue;
			}

			int i{ 0 };
			STL::Vector<std::pair<STL::Filepath, EFileAction>> parsedInformation;
			while(i < length)
			{
				const inotify_event* const event{ reinterpret_cast<inotify_event*>(&buffer[i]) };
				parsedInformation.emplace_back(event->name, [event]() -> EFileAction 
				{
					if(event->mask & IN_CREATE)
						return EFileAction::Created;

					if(event->mask & IN_MODIFY)
						return EFileAction::Modified;

					if(event->mask & IN_DELETE)
						return EFileAction::Deleted;

					return EFileAction::None;
				}());

				i += sizeof(inotify_event) + event->len;
			}

			[[likely]]	
			for(const auto& file : parsedInformation)
				m_Callback(file.first, file.second);		
		}

		[[maybe_unused]] auto _result = inotify_rm_watch(m_InotifyHandle, m_InotifyWatcherDescriptor) != -1;
	}
}
#endif