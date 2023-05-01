#ifdef CIN_PLATFORM_WINDOWS
#include "Cinnamon/include/Core/Core.h"

/* For UUIDS */
#include <Rpcdce.h>
#pragma comment(lib, "Rpcrt4.lib")

constexpr WORD C_FOREGROUND_BLACK = 0;
constexpr WORD C_FOREGROUND_BLUE = 1;
constexpr WORD C_FOREGROUND_GREEN = 2;
constexpr WORD C_FOREGROUND_CYAN = 3;
constexpr WORD C_FOREGROUND_RED = 4;
constexpr WORD C_FOREGROUND_MAGENTA = 5;
constexpr WORD C_FOREGROUND_BROWN = 6;
constexpr WORD C_FOREGROUND_LIGHTGRAY = 7;
constexpr WORD C_FOREGROUND_GRAY = 8;
constexpr WORD C_FOREGROUND_LIGHTBLUE = 9;
constexpr WORD C_FOREGROUND_LIGHTGREEN = 10;
constexpr WORD C_FOREGROUND_LIGHTCYAN = 11;
constexpr WORD C_FOREGROUND_LIGHTRED = 12;
constexpr WORD C_FOREGROUND_LIGHTMAGENTA = 13;
constexpr WORD C_FOREGROUND_YELLOW = 14;
constexpr WORD C_FOREGROUND_WHITE = 15;
constexpr WORD C_FOREGROUND_INTENSITY = 0x0008;

constexpr WORD C_BACKGROUND_BLACK = 0x0000;
constexpr WORD C_BACKGROUND_BLUE = 0x0010;
constexpr WORD C_BACKGROUND_GREEN = 0x0020;
constexpr WORD C_BACKGROUND_CYAN = 0x0030;
constexpr WORD C_BACKGROUND_RED = 0x0040;
constexpr WORD C_BACKGROUND_MAGENTA = 0x0050;
constexpr WORD C_BACKGROUND_YELLOW = 0x0060;
constexpr WORD C_BACKGROUND_GREY = 0x0070;
constexpr WORD C_BACKGROUND_INTENSITY = 0x0080;

namespace Cinnamon {
	struct
	{
		/* Time */
		double ClockFrequency{ 0.0 };
		LARGE_INTEGER ClockStartTime{ 0 };

		/* Console output */
		HANDLE StandardOutput{ nullptr };
		HANDLE StandardInput{ nullptr };
		HANDLE StandardError{ nullptr };

		CONSOLE_SCREEN_BUFFER_INFO DefaultConsoleBufferSpecification{};
	} constinit static s_PlatformState{};

	struct DistinctAttributes
	{
		WORD First;
		WORD Second;
		WORD Third;
		WORD Fourth;

		explicit DistinctAttributes(
			const WORD _1,
			const WORD _2,
			const WORD _3,
			const WORD _4) noexcept
			:
			First(_1),
			Second(_2),
			Third(_3),
			Fourth(_4)
		{}
	};

	InternalScope WORD GetTextAttributes(const CONSOLE_SCREEN_BUFFER_INFO& specification)
	{
		return specification.wAttributes;
	}

	InternalScope DistinctAttributes GetDistinctAttributes(const CONSOLE_SCREEN_BUFFER_INFO& specification)
	{
		const WORD attributes{ GetTextAttributes(specification) };
		return DistinctAttributes{
			static_cast<WORD>(attributes & C_FOREGROUND_GRAY),
			static_cast<WORD>(attributes & C_FOREGROUND_INTENSITY),
			static_cast<WORD>(attributes & C_BACKGROUND_GREY),
			static_cast<WORD>(attributes & C_BACKGROUND_INTENSITY),
		};
	}

	class ScopedOutputColor
	{
	public:
		explicit ScopedOutputColor(HANDLE handle, WORD defaultAttributes, WORD color) noexcept
			:
			m_Handle(handle),
			m_DefaultAttributes(defaultAttributes)
		{
			const auto distinct{ GetDistinctAttributes(s_PlatformState.DefaultConsoleBufferSpecification) };

			SetConsoleTextAttribute(
				s_PlatformState.StandardOutput,
				static_cast<WORD>(distinct.Third | distinct.Fourth | color | C_FOREGROUND_INTENSITY));
		}

		~ScopedOutputColor() noexcept
		{
			SetConsoleTextAttribute(
				s_PlatformState.StandardOutput,
				m_DefaultAttributes);
		}
	private:
		const HANDLE m_Handle;
		const WORD m_DefaultAttributes;
	};

	namespace Platform {
		Errr Initialize()
		{
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			s_PlatformState.ClockFrequency = 1.0 / static_cast<double>(frequency.QuadPart);
			QueryPerformanceCounter(&s_PlatformState.ClockStartTime);
#ifndef CIN_DISTRIBUTION
			s_PlatformState.StandardOutput = GetStdHandle(STD_OUTPUT_HANDLE);
			s_PlatformState.StandardInput = GetStdHandle(STD_INPUT_HANDLE);
			s_PlatformState.StandardError = GetStdHandle(STD_ERROR_HANDLE);

			if (not s_PlatformState.StandardOutput or not s_PlatformState.StandardInput or not s_PlatformState.StandardError)
			{
				/* TODO: Create new ones? */
				return Error::Failure;
			}

			if (!GetConsoleScreenBufferInfo(
				s_PlatformState.StandardOutput,
				&s_PlatformState.DefaultConsoleBufferSpecification))
			{
				/* TODO: Handle? */
				return Error::Failure;
			}
#endif
			return Error::Success;
		}

		void Shutdown()
		{}
			
		STL::String GetBuildDate()
		{
			return CIN_TIMESTAMP;
		}

		STL::String GenerateUUID()
		{
			UUID uuid;
			RPC_CSTR szUuid{ NULL };

			(void)UuidCreate(&uuid);
			(void)UuidToStringA(&uuid, &szUuid);

			const std::string stringUUID{ std::move(std::string(reinterpret_cast<char*>(szUuid))) };
			return stringUUID;
		}

		void WriteToConsole(const char* message, const EConsoleTextColor color)
		{
#ifdef CIN_DISTRIBUTION
			/* Move to logger? This should not be ever called in distribution */
			const HANDLE dumpFile
			{
				CreateFile
				(
					"dump.txt",
					GENERIC_WRITE,
					0,
					nullptr,
					OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					nullptr
				)
			};

			[[unlikely]]
			if (dumpFile == INVALID_HANDLE_VALUE)
				CIN_PANIC_EXIT();

			CHAR dumpMessage[]{ "Attempted trying to write to console in distribution mode." };
			WriteFile(
				dumpFile,
				dumpMessage,
				sizeof(dumpMessage) / sizeof(message[0]),
				0,
				nullptr);

			CloseHandle(
				dumpFile);
			CIN_PANIC_EXIT();
#endif
			/* Reflected off EConsoleTextColor */
			constexpr WORD colors[5]{
				C_FOREGROUND_WHITE,
				C_FOREGROUND_GRAY,
				C_FOREGROUND_YELLOW,
				C_FOREGROUND_RED,
				C_FOREGROUND_MAGENTA };

			/* Send to debugger */
			OutputDebugStringA(message);
			{
				ScopedOutputColor scoped{
					s_PlatformState.StandardOutput,
					s_PlatformState.DefaultConsoleBufferSpecification.wAttributes,
					colors[static_cast<std::size_t>(color)] };

				WriteConsoleA(
					s_PlatformState.StandardOutput,
					message,
					static_cast<DWORD>(strlen(message)),
					NULL,
					NULL);
			}
		}

		double GetAbsoluteTime()
		{
			LARGE_INTEGER currentTime;
			QueryPerformanceCounter(&currentTime);

			return static_cast<double>(currentTime.QuadPart) * s_PlatformState.ClockFrequency;
		}

		STL::Vector<const char*> GetRequiredVulkanExtensions()
		{
			return 
			{
				"VK_KHR_win32_surface",
				"VK_KHR_surface",
				"VK_KHR_get_physical_device_properties2",
				/* "VK_KHR_bind_memory2", core in 1.1 */
				/* VK_KHR_dedicated_allocation, core in 1.1 */
#ifdef CIN_DEBUG
				"VK_EXT_debug_report",
				"VK_EXT_debug_utils",
#endif
			};
		}

		STL::Vector<const char*> GetRequestedVulkanLayers()
		{
			return 
			{
#ifdef CIN_DEBUG
				"VK_LAYER_KHRONOS_validation",
#endif
			};
		}

		STL::Vector<const char*> GetRequiredVulkanDeviceExtensions()
		{
			return 
			{
				"VK_KHR_swapchain",
			};
		}

		STL::Vector<const char*> GetRequestedVulkanDeviceLayers()
		{
			return {};
		}
	}
}
#endif