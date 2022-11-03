#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Core/Filesystem.h"

namespace Cinnamon {
	static constexpr std::size_t g_BufferSize = { 2 << 16 };
	constexpr DWORD g_ListenFilters
	{
		FILE_NOTIFY_CHANGE_SECURITY		|
		FILE_NOTIFY_CHANGE_CREATION		|
		FILE_NOTIFY_CHANGE_LAST_ACCESS	|
		FILE_NOTIFY_CHANGE_LAST_WRITE	|
		FILE_NOTIFY_CHANGE_SIZE			|
		FILE_NOTIFY_CHANGE_ATTRIBUTES	|
		FILE_NOTIFY_CHANGE_DIR_NAME		|
		FILE_NOTIFY_CHANGE_FILE_NAME
	};

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
		m_CallbackInformation{},
#ifdef CIN_PLATFORM_WINDOWS
		m_DirectoryHandle(nullptr),
		m_QuitWatchingEvent(nullptr)
#endif
	{
		CIN_ASSERT(FileExists(path)	and IsDirectory(path));
		m_QuitWatchingEvent = CreateEvent(nullptr, true, false, nullptr);
		CIN_ASSERT(m_QuitWatchingEvent);
		m_CallbackThread = std::move(std::thread(&FileWatcher::CallbackThreadWork, this));
		m_WatchThread = std::move(std::thread(&FileWatcher::WatcherThreadWork, this));

		std::future<void> future = m_IsSetup.get_future();
		future.get();
	}

	FileWatcher::~FileWatcher() noexcept
	{
		m_IsWatching = false;
		m_IsSetup = std::promise<void>();
		
		CIN_VERIFY(SetEvent(
			m_QuitWatchingEvent));
		
		m_NewCallbackInformation.notify_all();
		m_WatchThread.join();
		m_CallbackThread.join();
		
		CIN_VERIFY(CloseHandle(
			m_DirectoryHandle));
	}

	void FileWatcher::WatcherThreadWork()
	{
		const DWORD fileInfo{ GetFileAttributesA(m_ObservedPath.c_str()) };
		CIN_ASSERT(fileInfo != INVALID_FILE_ATTRIBUTES);

		m_DirectoryHandle = CreateFileA
		(
			m_ObservedPath.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			nullptr
		);
		CIN_ASSERT(m_DirectoryHandle != INVALID_HANDLE_VALUE);

		DWORD bytesReturned{ 0 };
		OVERLAPPED overlappedBuffer
		{
			.Internal{ 0 },
			.InternalHigh{ 0 },
			.Pointer{ nullptr },
			.hEvent{ CreateEvent(nullptr, true, false, nullptr) },
		};

		CIN_ASSERT(overlappedBuffer.hEvent);
		static thread_local STL::Array<std::byte, g_BufferSize> buffer;
		const STL::Array<HANDLE, 2> handles{ overlappedBuffer.hEvent, m_QuitWatchingEvent };
		bool asyncIOPending = false;
		/* Work is setup */
		m_IsSetup.set_value();
		[[likely]]
		while (m_IsWatching)
		{
			STL::Vector<std::pair<Filepath, EFileAction>> parsedInformation;
			CIN_VERIFY(ReadDirectoryChangesW(
				m_DirectoryHandle,
				reinterpret_cast<LPVOID>(buffer.data()),
				static_cast<DWORD>(buffer.size()),
				true,
				g_ListenFilters,
				&bytesReturned,
				&overlappedBuffer,
				nullptr));

			asyncIOPending = true;
			switch (WaitForMultipleObjects(
				static_cast<DWORD>(handles.size()),
				handles.data(),
				false,
				INFINITE))
			{
				case WAIT_OBJECT_0:
				{
					CIN_VERIFY(GetOverlappedResult(
						m_DirectoryHandle,
						&overlappedBuffer,
						&bytesReturned,
						TRUE));

					asyncIOPending = false;
					if (bytesReturned == 0)
						break;

					FILE_NOTIFY_INFORMATION* fileInformation{ reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data()) };
					[[likely]]
					while (true)
					{
						const STL::WString changedFileNameWide{ fileInformation->FileName, fileInformation->FileNameLength / sizeof(fileInformation->FileName[0]) };
						const STL::String changedFileName
						{
							[](const STL::WString& wideString)
							{
								const int sizeNeeded{  WideCharToMultiByte(CP_UTF8, 0, &wideString[0], static_cast<int>(wideString.size()), NULL, 0, NULL, NULL) };
								STL::String out(sizeNeeded, '\0');
								WideCharToMultiByte(CP_UTF8, 0, &wideString[0], static_cast<int>(wideString.size()), &out[0], sizeNeeded, NULL, NULL);

								return out;
							}(changedFileNameWide)
						};

						const STL::String extension{ STL::Filepath(changedFileName).extension().string() };
						if (std::find_if(m_ObservedExtensions.cbegin(), m_ObservedExtensions.cend(), [&extension](const STL::String& observedExtension) 
							{
								return observedExtension == extension;
							}) != m_ObservedExtensions.cend())
						{
							parsedInformation.emplace_back(changedFileName, [](const DWORD action)
								{
									switch (action)
									{
										case FILE_ACTION_ADDED:				return EFileAction::Created;
										case FILE_ACTION_REMOVED:			return EFileAction::Deleted;
										case FILE_ACTION_MODIFIED:			return EFileAction::Modified;
										case FILE_ACTION_RENAMED_OLD_NAME:	return EFileAction::RenamedOld;
										case FILE_ACTION_RENAMED_NEW_NAME:	return EFileAction::RenamedNew;
										default:							CIN_ASSERT(false); break;
									}

									CIN_ASSERT(false);
									return EFileAction::None;
								}(fileInformation->Action));
						}

						if (not fileInformation->NextEntryOffset)
							break;
						else
							fileInformation = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(fileInformation) + fileInformation->NextEntryOffset);
					}

					break;
				}

				case WAIT_OBJECT_0 + 1:
				{
					/* Quits */
					[[maybe_unused]] int x{ 0 };
					break;
				}

				case WAIT_FAILED:
				{
					break;
				}
			}

			{
				[[maybe_unused]] const std::lock_guard<STL::Mutex> lock(m_CallbackMutex);
				m_CallbackInformation.insert(m_CallbackInformation.end(), parsedInformation.begin(), parsedInformation.end());
			}
			m_NewCallbackInformation.notify_all();
		}

		[[likely]]
		if (asyncIOPending)
		{
			CIN_VERIFY(CancelIo(
				m_DirectoryHandle));
			
			GetOverlappedResult(
				m_DirectoryHandle, 
				&overlappedBuffer, 
				&bytesReturned, 
				TRUE);
		}
	}

	void FileWatcher::CallbackThreadWork()
	{
		[[likely]]
		while (m_IsWatching)
		{
			std::unique_lock<std::mutex> lock(m_CallbackMutex);
			STL::Vector<std::pair<Filepath, EFileAction>> callbackInformation{};
			{

				if (m_CallbackInformation.empty() and m_IsWatching)
				{
					m_NewCallbackInformation.wait(lock, [this]()
						{
							return not m_CallbackInformation.empty() or not m_IsWatching;
						});
				}

				std::swap(callbackInformation, m_CallbackInformation);
				lock.unlock();
			}

			[[likely]]
			for (const auto& file : callbackInformation)
				m_Callback(file.first, file.second);
		}
	}
}
#endif