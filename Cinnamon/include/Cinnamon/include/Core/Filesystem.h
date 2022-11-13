#pragma once
#include "Cinnamon/include/Core/Core.h"
#include "Cinnamon/include/Core/CinSTL.h"

namespace Cinnamon {
	CIN_FORCE_INLINE bool FileExists(const STL::Filepath& path) noexcept
	{
		return std::filesystem::exists(path);
	}

	CIN_FORCE_INLINE bool IsDirectory(const STL::Filepath& path) noexcept
	{
		CIN_ASSERT(FileExists(path));
		return std::filesystem::is_directory(path);
	}

	CIN_FORCE_INLINE bool IsRegularFile(const STL::Filepath& path) noexcept
	{
		CIN_ASSERT(FileExists(path));
		return std::filesystem::is_regular_file(path);
	}

	CIN_FORCE_INLINE STL::LastFileWriteStamp FileLastWrittenTo(const STL::Filepath& path) noexcept
	{
		CIN_ASSERT(FileExists(path));
		return std::filesystem::last_write_time(path);
	}

	enum class EFileAction
	{
		None,
		Created,
		Deleted,
		Modified,
		RenamedOld,
		RenamedNew,
	};

	class FileWatcher
	{
	private:
		NON_MOVABLE(FileWatcher)
		NON_COPYABLE(FileWatcher)
	public:
		using FilepathT = STL::String;
		using FileWatcherCallback = void (*)(const FilepathT, const EFileAction) noexcept;
	public:
		explicit FileWatcher(const FilepathT& path, const STL::InitializerList<STL::String>& observedExtensions, FileWatcherCallback callback) noexcept;
		~FileWatcher() noexcept;
	private:
		void WatcherThreadWork() noexcept;
	private:
		mutable std::atomic<bool> m_IsWatching;
		const FilepathT m_ObservedPath;
		const FileWatcherCallback m_Callback;
		const STL::Vector<STL::String> m_ObservedExtensions;

		std::thread m_WatchThread;
		/* Signaled when watcher thread is properly setup */
		std::promise<void> m_IsSetup;
#ifdef CIN_PLATFORM_WINDOWS
		HANDLE m_DirectoryHandle;
		/* Fired when closing */
		HANDLE m_QuitWatchingEvent;
#endif
	};
}