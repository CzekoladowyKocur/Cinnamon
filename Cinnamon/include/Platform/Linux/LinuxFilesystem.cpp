#include "Cinnamon/include/Core/TypeDefines.hpp"
#ifdef CIN_PLATFORM_LINUX
#include "Cinnamon/include/Core/Filesystem.hpp"
#include <sys/inotify.h>
#include <sys/inotify.h>
#include <poll.h>

namespace Cinnamon {
	/* TODO: Temporary */
	bool PlatformForceLinking{ false };

	constexpr uint32_t s_RootWatcherFlags
	{
	    IN_CREATE           | 
	    IN_DELETE           | 
	    IN_MODIFY           | 
	    IN_MOVED_FROM       | 
	    IN_MOVED_TO         | 
	    IN_DELETE_SELF      | 
	    IN_MOVE_SELF
	};

	struct FileWatcherInternalState
	{
	    // Inotify instance handle
	    int InotifyInstance{ -1 };
	    // Root directory watch descriptor
	    int RootWatchDescriptor{ -1 };
	    // Subdirectory watch descriptors
	    std::unordered_map<int, std::filesystem::path> SubdirectoryWatchDescriptors{};
	};

	FileWatcher::FileWatcher(const std::filesystem::path& observedPath, FileWatcherCallback&& callback, const bool returnAbsolutePath, std::error_code& error) noexcept
	    :
		m_IsWatching(false),
		m_ObservedPath(observedPath),
		m_Callback(std::move(callback)),
		m_WatcherThread{},
		m_InternalState(nullptr)
	{
	    CIN_ASSERT(m_Callback != nullptr);
	    SetupWatcher(returnAbsolutePath, error);
	}

	FileWatcher::FileWatcher(const std::filesystem::path& observedPath, const FileWatcherCallback& callback, const bool returnAbsolutePath, std::error_code& error) noexcept
	    :
		m_IsWatching(false),
		m_ObservedPath(observedPath),
		m_Callback(std::move(callback)),
		m_WatcherThread{},
		m_InternalState(nullptr)
	{
	    CIN_ASSERT(m_Callback != nullptr);
	    SetupWatcher(returnAbsolutePath, error);
	}

	FileWatcher::~FileWatcher() noexcept
	{
	    m_IsWatching = false;
	
	    if(m_InternalState && m_InternalState->RootWatchDescriptor != -1)
	    {
	        CIN_ASSERT(m_InternalState->InotifyInstance != -1);
	        inotify_rm_watch(m_InternalState->InotifyInstance, m_InternalState->RootWatchDescriptor);
	    }

	    if(m_WatcherThread.joinable())
	        m_WatcherThread.join();

	    if(m_InternalState && m_InternalState->InotifyInstance != -1)
	        close(m_InternalState->InotifyInstance);
	}

	bool FileWatcher::IsWatching() const noexcept
	{
	    return m_IsWatching.load();
	}

	void FileWatcher::SetupWatcher(const bool useAsbolutePath, std::error_code& error) noexcept
	{
    	if (!std::filesystem::exists(m_ObservedPath))
		{
			if (m_ObservedPath.has_parent_path() && m_ObservedPath.has_filename())
			{
				m_ObservedFile = m_ObservedPath.filename();
				m_ObservedPath = m_ObservedPath.parent_path();
			}
			else
			{
				error.assign(static_cast<int>(EFileWatcherError::SpecifiedFileDoesntExist), FileWatcherCategory());

				return;
			}
		}

		if (std::filesystem::is_regular_file(m_ObservedPath))
		{
			if (m_ObservedPath.has_parent_path())
			{
				if (m_ObservedPath.has_filename())
				{
					m_ObservedFile = m_ObservedPath.filename();
					m_ObservedPath = m_ObservedPath.parent_path();
				}
				else
				{
               	 error.assign(static_cast<int>(EFileWatcherError::InvalidFile), FileWatcherCategory());
					return;
				}
			}
			else
			{
        	    error.assign(static_cast<int>(EFileWatcherError::RegularFileHasNoParentDirectory), FileWatcherCategory());
				return;
			}
		}

		if (useAsbolutePath)
		{
			m_ObservedPath = std::filesystem::absolute(m_ObservedPath, error);
			if (error)
				return;
		}

    	const int inotifyInstance{ inotify_init1(IN_NONBLOCK) };
    	if(inotifyInstance == -1)
    	{
    	    error.assign(errno, std::system_category());
			return;
    	}

    	const std::string path{ m_ObservedPath.string() };
    	const int watcherHandle{ inotify_add_watch(inotifyInstance, path.c_str(), s_RootWatcherFlags) }; 
	
    	if(watcherHandle == -1)
    	{
    	    error.assign(errno, std::system_category());
    	    close(inotifyInstance);
			return;
    	}

    	m_InternalState = std::make_unique<FileWatcherInternalState>();
    	m_InternalState->InotifyInstance = inotifyInstance;
    	m_InternalState->RootWatchDescriptor = watcherHandle;
    	m_IsWatching = true;

    	if(m_ObservedFile.empty())
    	{
    	    for(const auto& file : std::filesystem::recursive_directory_iterator(m_ObservedPath))
    	    {
    	        if(file.is_directory())
    	        {
    	            const int subdirectoryWatchHandle{ inotify_add_watch(m_InternalState->InotifyInstance, std::filesystem::path(file).string().c_str(), s_RootWatcherFlags) };     
    	            if(subdirectoryWatchHandle != -1)
    	                m_InternalState->SubdirectoryWatchDescriptors[subdirectoryWatchHandle] = file;
    	            else
    	            {
    	                m_IsWatching = false;
    	                error.assign(errno, std::system_category());
    	                break;
    	            }
    	        }
    	    }
    	}

    	m_WatcherThread = std::thread(&FileWatcher::WatcherThreadWork, this);
	}

	void FileWatcher::WatcherThreadWork() const noexcept
	{
    	std::byte* watchBuffer{ reinterpret_cast<std::byte*>(malloc(static_cast<int>(s_WatchBufferSize))) };  
    
    	// cookie -> old file name
    	std::unordered_map<uint32_t, std::filesystem::path> renamedFiles;

    	if(!watchBuffer)
    	    goto quitMonitoring;

    	while(m_IsWatching) [[likely]]
    	{
        	pollfd fileEventReadPoll
        	{
        	   .fd{ m_InternalState->InotifyInstance },
        	   .events{ POLLRDNORM },
        	   .revents{}
        	};

        	// Polling avoids thread exhaustion
        	switch(poll(&fileEventReadPoll, 1, -1))
        	{
            	case -1:
            	{
            	    m_Callback({}, std::nullopt, EFileAction::Error, std::error_code(errno, std::system_category()));
            	    goto quitMonitoring;
            	} break;
            
            	default:
            	{
            	    const bool readAvailable{ static_cast<bool>(fileEventReadPoll.revents & POLLRDNORM) };
	
            	    [[unlikely]]
            	    if(!readAvailable) // Should never happen
            	        continue;
            	} break;
       		}       

        	usleep(500); // Sleep 500 microseconds to reinforce IN_MOVED_FROM + IN_MOVED_TO pair as they are not atomic
        	const int length{ static_cast<int>(read(m_InternalState->InotifyInstance, watchBuffer, s_WatchBufferSize)) };
        	if(length == -1)
        	{
        	   	m_Callback({}, std::nullopt, EFileAction::Error, std::error_code(errno, std::system_category()));
            	goto quitMonitoring;
        	}

        	int i{ 0 };
        	while(i < length)
        	{
            	const inotify_event* const event{ reinterpret_cast<inotify_event*>(&watchBuffer[i]) };
            	i += (sizeof(inotify_event) + event->len);

            	if(
                	event->mask & IN_IGNORED        || 
                	event->mask & IN_DELETE_SELF    || 
                	event->mask & IN_MOVE_SELF)     // Watched directory was deleted, renamed or the filesystem was unmounted.
            	{
                	if(m_InternalState->RootWatchDescriptor == event->wd && m_IsWatching)
                	{
                    	m_Callback({}, std::nullopt, EFileAction::Error, std::error_code(static_cast<int>(EFileWatcherError::WatchedDirectoryWasDeleted), FileWatcherCategory()));
                    	goto quitMonitoring;
                	}
                	else if (m_InternalState->SubdirectoryWatchDescriptors.contains(event->wd)) 
                	{
                    	inotify_rm_watch(m_InternalState->RootWatchDescriptor, event->wd);
                    	m_InternalState->SubdirectoryWatchDescriptors.erase(event->wd);
                	}
            	}

            	if(event->len) // Length will be 0 if watch was removed.
            	{
                	// A file was created. If the subject is a directory, we add a watch to keep track of it's contents.
                	if(event->mask & IN_CREATE)
                	{
                    	std::filesystem::path file{ ConstructReturnPath((struct FilewatcherCharacterType*)event->name, event->wd) };
                    
                    	if(event->mask & IN_ISDIR)
                    	{
                        	const int subdirectoryWatchHandle{ inotify_add_watch(m_InternalState->InotifyInstance, file.string().c_str(), s_RootWatcherFlags) };     
                        
                        	if(subdirectoryWatchHandle != -1)
	                            m_InternalState->SubdirectoryWatchDescriptors[subdirectoryWatchHandle] = file;
    	                    else
        	                    m_Callback(file, std::nullopt, EFileAction::Error, std::error_code(errno, std::system_category()));
            	        }

                	    if(m_ObservedFile.empty() || m_ObservedFile == file.filename())
                    	    m_Callback(std::move(file), std::nullopt, EFileAction::Created, std::error_code{});
                	}

    	            if(event->mask & IN_DELETE)
        	        {
                    	std::filesystem::path file{ ConstructReturnPath((struct FilewatcherCharacterType*)event->name, event->wd) };
                    
	                    if(m_ObservedFile.empty() || m_ObservedFile == file.filename())
    	                   m_Callback(std::move(file), std::nullopt, EFileAction::Deleted, std::error_code{});
        	        }

            	    if(event->mask & IN_MODIFY)
                	{
	                    std::filesystem::path file{ ConstructReturnPath((struct FilewatcherCharacterType*)event->name, event->wd) };
    	                if(m_ObservedFile.empty() || m_ObservedFile == file.filename())
        	                m_Callback(std::move(file), std::nullopt, EFileAction::Modified, std::error_code{});
            	    }

    	            if(event->mask & IN_MOVED_FROM)
        	            renamedFiles[event->cookie] = ConstructReturnPath((struct FilewatcherCharacterType*)event->name, event->wd);

    	            if(event->mask & IN_MOVED_TO)
        	        {
            	        std::filesystem::path file{ ConstructReturnPath((struct FilewatcherCharacterType*)event->name, event->wd) };
                	    if(m_ObservedFile.empty() || m_ObservedFile == file.filename() || renamedFiles.contains(event->cookie))
                    	{
                        	m_Callback(renamedFiles[event->cookie], std::move(file), EFileAction::Renamed, std::error_code{});
	                        renamedFiles.erase(event->cookie);
    	                }
        	        }
	            }
    	    }
    	}

	quitMonitoring:
    	CIN_ASSERT(renamedFiles.empty());

	    for(auto&& [watcherDescriptor, _] : m_InternalState->SubdirectoryWatchDescriptors)
    	{
        	CIN_ASSERT(watcherDescriptor != -1);
        	inotify_rm_watch(m_InternalState->InotifyInstance, watcherDescriptor);    
	    }

    	m_InternalState->SubdirectoryWatchDescriptors.clear();
    	m_IsWatching = false;

	    [[likely]]
	    if(watchBuffer)
    	    free(watchBuffer);
	}

	struct alignas(alignof(char)) FilewatcherCharacterType { char Character; };
	static_assert(sizeof(FilewatcherCharacterType) == sizeof(char) && alignof(FilewatcherCharacterType) == alignof(char));

	std::filesystem::path FileWatcher::ConstructReturnPath(struct FilewatcherCharacterType* fileNameBuffer, const size_t watcherDescriptor) const noexcept
	{
    	const std::string fileName(reinterpret_cast<char*>(fileNameBuffer));
    	if(m_InternalState->RootWatchDescriptor != static_cast<int>(watcherDescriptor))
	        return m_InternalState->SubdirectoryWatchDescriptors[static_cast<int>(watcherDescriptor)] / fileName;
    	else
        	return m_ObservedPath / fileName;
	}	
}
#endif