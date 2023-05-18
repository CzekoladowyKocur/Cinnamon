#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Core/Filesystem.hpp"

namespace Cinnamon {
	struct FileWatcherInternalState
	{
		explicit FileWatcherInternalState(const size_t watchBufferSize, const HANDLE observedFileHandle) noexcept
			:
			WatchBuffer{},
			ObservedFileHandle{ observedFileHandle },
			OverlappedBuffer{}
		{
			WatchBuffer.resize(watchBufferSize);
		}

		std::wstring WatchBuffer;
		HANDLE ObservedFileHandle;
		OVERLAPPED OverlappedBuffer;
		HANDLE QuitWatchingEvent;
	};

	FileWatcher::FileWatcher(const STL::Filepath& observedPath, FileWatcherCallback&& callback, const bool returnAbsolutePath, STL::ErrorCode& error) noexcept
		:
		m_IsWatching(false),
		m_ObservedPath(observedPath),
		m_Callback(std::move(callback)),
		m_WatcherThread{},
		m_InternalState(nullptr)
	{
		CIN_ASSERT(m_Callback);
		SetupWatcher(returnAbsolutePath, error);
	}

	FileWatcher::FileWatcher(const STL::Filepath& observedPath, const FileWatcherCallback& callback, const bool returnAbsolutePath, STL::ErrorCode& error) noexcept
		:
		m_IsWatching(false),
		m_ObservedPath(observedPath),
		m_Callback(callback),
		m_WatcherThread{},
		m_InternalState(nullptr)
	{
		CIN_ASSERT(m_Callback);
		SetupWatcher(returnAbsolutePath, error);
	}

	FileWatcher::~FileWatcher() noexcept
	{
		m_IsWatching = false;

		if (m_InternalState)
			if (m_InternalState->QuitWatchingEvent)
				SetEvent(m_InternalState->QuitWatchingEvent);

		if (m_WatcherThread.joinable())
			m_WatcherThread.join();

		if (m_InternalState)
			if (m_InternalState->ObservedFileHandle != INVALID_HANDLE_VALUE)
				CloseHandle(m_InternalState->ObservedFileHandle);

		m_Callback = nullptr;
	}

	bool FileWatcher::IsWatching() const noexcept
	{
		return m_IsWatching.load();
	}

	void FileWatcher::SetupWatcher(const bool returnAbsolutePath, STL::ErrorCode& error) noexcept
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
				error.assign(static_cast<int>(EFileWatcherError::SpecifiedFileDoesntExist), FileWatcherErrorCategory());
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
					error.assign(static_cast<int>(EFileWatcherError::InvalidFile), FileWatcherErrorCategory());
					return;
				}
			}
			else
			{
				error.assign(static_cast<int>(EFileWatcherError::RegularFileHasNoParentDirectory), FileWatcherErrorCategory());
				return;
			}
		}

		if (returnAbsolutePath)
		{
			STL::ErrorCode errorCode;
			m_ObservedPath = std::filesystem::absolute(m_ObservedPath, errorCode);

			if (errorCode)
				return;
		}

		const std::wstring observedPathWide{ m_ObservedPath.wstring() };
		const HANDLE observedFileHandle
		{
			CreateFileW
			(
				static_cast<LPCWSTR>(observedPathWide.data()),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr,
				OPEN_EXISTING, /* Open only existing files */
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				nullptr
			)
		};

		if (observedFileHandle == INVALID_HANDLE_VALUE)
		{
			error.assign(static_cast<int>(GetLastError()), std::system_category());
			return;
		}

		m_InternalState = STL::MakeUnique<FileWatcherInternalState>(s_WatchBufferSize, observedFileHandle);
		if (!m_InternalState)
		{
			error.assign(static_cast<int>(EFileWatcherError::InternalStateCreationFailed), FileWatcherErrorCategory());
			return;
		}

		ZeroMemory(&m_InternalState->OverlappedBuffer, sizeof(m_InternalState->OverlappedBuffer));
		m_InternalState->OverlappedBuffer.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_InternalState->OverlappedBuffer.hEvent == INVALID_HANDLE_VALUE)
		{
			error.assign(static_cast<int>(GetLastError()), std::system_category());
			return;
		}

		m_InternalState->QuitWatchingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_InternalState->QuitWatchingEvent == INVALID_HANDLE_VALUE)
		{
			error.assign(static_cast<int>(GetLastError()), std::system_category());
			return;
		}

		m_IsWatching = true;
		m_WatcherThread = std::move(std::thread(&FileWatcher::WatcherThreadWork, this));
	}

	void FileWatcher::WatcherThreadWork() const noexcept
	{
		/* Used later for managing callbacks */
		STL::Filepath renamedOld;
		STL::Filepath previouslyCreatedFile;
		EFileAction previousFileAction{ EFileAction::Error };

	beginWork:
		[[likely]]
		while (m_IsWatching)
		{
			const BOOL success
			{
				ReadDirectoryChangesW
				(
					m_InternalState->ObservedFileHandle,
					static_cast<LPVOID>(m_InternalState->WatchBuffer.data()),
					static_cast<DWORD>(m_InternalState->WatchBuffer.size()),
					m_ObservedFile.empty() ? TRUE : FALSE, /* Recursive only if observing a directory */
					FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
					0,
					&m_InternalState->OverlappedBuffer,
					0
				)
			};

			// If the function succeeds, the return value is nonzero. For synchronous calls, this means that the operation succeeded
			if (!success)
			{
				m_Callback(STL::Filepath{}, std::nullopt, EFileAction::Error, STL::ErrorCode{ static_cast<int>(GetLastError()), std::system_category() });
				goto beginWork;
			}

			const HANDLE synchronizationObjects[2U]{ m_InternalState->OverlappedBuffer.hEvent, m_InternalState->QuitWatchingEvent };
			switch (WaitForMultipleObjects
			(
				sizeof(synchronizationObjects) / sizeof(synchronizationObjects[0U]),
				synchronizationObjects,
				FALSE, // Proceed if an overlapped event had happened or file watcher was suspended
				INFINITE
			))
			{
				/* Overlapped event */
			case WAIT_OBJECT_0:
			{
				DWORD readBytes{ 0U };
				const BOOL result
				{
					GetOverlappedResult
					(
						m_InternalState->ObservedFileHandle,
						&m_InternalState->OverlappedBuffer,
						&readBytes,
						TRUE	/* Put thread to sleep */
					)
				};

				// If the function succeeds, the return value is nonzero. If the function fails, the return value is zero.
				if (!result)
				{
					m_Callback(STL::Filepath{}, std::nullopt, EFileAction::Error, STL::ErrorCode(static_cast<int>(GetLastError()), std::system_category()));
					goto beginWork;
				}

				const FILE_NOTIFY_INFORMATION* event{ reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_InternalState->WatchBuffer.data()) };
				do
				{
					switch (event->Action)
					{
					case FILE_ACTION_ADDED:
					{
						STL::Filepath file(ConstructReturnPath(reinterpret_cast<struct FilewatcherCharacterType*>(const_cast<wchar_t*>((event->FileName))), event->FileNameLength));
						if (m_ObservedFile.empty() || m_ObservedFile == file.filename())
							m_Callback(std::move(file), std::nullopt, EFileAction::Created, STL::ErrorCode{});

						previousFileAction = EFileAction::Created;
					} break;

					case FILE_ACTION_REMOVED:
					{
						STL::Filepath file = previouslyCreatedFile = (ConstructReturnPath(reinterpret_cast<struct FilewatcherCharacterType*>(const_cast<wchar_t*>((event->FileName))), event->FileNameLength));
						if (m_ObservedFile.empty() || m_ObservedFile == file.filename())
							m_Callback(std::move(file), std::nullopt, EFileAction::Deleted, STL::ErrorCode{});

					} break;

					case FILE_ACTION_MODIFIED:
					{
						/* Skip "modification" if file was just created */
						STL::Filepath file(ConstructReturnPath(reinterpret_cast<struct FilewatcherCharacterType*>(const_cast<wchar_t*>((event->FileName))), event->FileNameLength));
						if (previouslyCreatedFile == file && previousFileAction == EFileAction::Created)
						{
							previousFileAction = EFileAction::Modified;
							break;
						}

						if (m_ObservedFile.empty() || m_ObservedFile == file.filename())
							m_Callback(std::move(file), std::nullopt, EFileAction::Modified, STL::ErrorCode{});

					} break;

					case FILE_ACTION_RENAMED_OLD_NAME:
					{
						renamedOld = ConstructReturnPath(reinterpret_cast<struct FilewatcherCharacterType*>(const_cast<wchar_t*>((event->FileName))), event->FileNameLength);
						previousFileAction = EFileAction::Renamed;
					} break;

					case FILE_ACTION_RENAMED_NEW_NAME:
					{
						STL::Filepath file(ConstructReturnPath(reinterpret_cast<struct FilewatcherCharacterType*>(const_cast<wchar_t*>((event->FileName))), event->FileNameLength));
						if (m_ObservedFile.empty() || m_ObservedFile == file.filename() || m_ObservedFile == renamedOld.filename())
							m_Callback(renamedOld, std::move(file), EFileAction::Renamed, STL::ErrorCode{});

					} break;
					}

					if (event->NextEntryOffset == 0U)
						break;
					else
						event = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(const_cast<FILE_NOTIFY_INFORMATION*>(event)) + event->NextEntryOffset);
				} while (true);
			} break;

			case WAIT_OBJECT_0 + 1U:
			{
				/* Quit */
				goto quitMonitoring;
			} break;

			case WAIT_FAILED:
			{
				/* Should not have happened */
				m_Callback(STL::Filepath{}, std::nullopt, EFileAction::Error, STL::ErrorCode{ static_cast<int>(GetLastError()), std::system_category() });
			} break;
			}
		}

	quitMonitoring:
		return;
	}

	struct alignas(alignof(wchar_t)) FilewatcherCharacterType { wchar_t Character; };
	static_assert(sizeof(FilewatcherCharacterType) == sizeof(wchar_t) && alignof(FilewatcherCharacterType) == alignof(wchar_t));

	STL::Filepath FileWatcher::ConstructReturnPath(struct FilewatcherCharacterType* fileName, const size_t fileNameLength) const noexcept
	{
		const size_t bufferSize{ fileNameLength + sizeof(wchar_t) };
		wchar_t* buffer{ reinterpret_cast<wchar_t*>(calloc(bufferSize / 2U, sizeof(wchar_t))) };

		[[likely]]
		if (buffer)
		{
			memcpy(buffer, fileName, fileNameLength);
			auto observedPath{ m_ObservedPath };
			observedPath.append(buffer);
			free(buffer);
			return observedPath;
		}

		return m_ObservedPath;
	}
}
#endif