#ifdef CIN_PLATFORM_WINDOWS
#include "Platform/Platform.h"
#include "Cinnamon/include/Core/Core.h"

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
		HANDLE StandardOutput{ nullptr };
		HANDLE StandardInput{ nullptr };
		HANDLE StandardError{ nullptr };

		CONSOLE_SCREEN_BUFFER_INFO DefaultConsoleBufferSpecification;
	} static s_PlatformState{};

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
				distinct.Third | distinct.Fourth | color | C_FOREGROUND_INTENSITY);
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

	bool Platform::Initialize()
	{
		s_PlatformState.StandardOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		s_PlatformState.StandardInput = GetStdHandle(STD_INPUT_HANDLE);
		s_PlatformState.StandardError = GetStdHandle(STD_ERROR_HANDLE);
	
		if (!s_PlatformState.StandardOutput or !s_PlatformState.StandardInput or !s_PlatformState.StandardError)
		{
			/* TODO: Create new ones */
			return false;
		}

		if (!GetConsoleScreenBufferInfo(s_PlatformState.StandardOutput, &s_PlatformState.DefaultConsoleBufferSpecification))
		{
			/* TODO: Handle */
			return false;
		}

		return true;
	}

	bool Platform::Shutdown()
	{
		return true;
	}

	double Platform::GetAbsoluteTime()
	{
		CIN_UNIMPLEMENTED();
		return 0.0;
	}

	/* Vulkan */
	STL::Vector<const char*> Platform::GetRequiredVulkanExtensions()
	{
		return {
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

	STL::Vector<const char*> Platform::GetRequestedVulkanLayers()
	{
		return {
#ifdef CIN_DEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
		};
	}

	STL::Vector<const char*> Platform::GetRequiredVulkanDeviceExtensions()
	{
		return {
			"VK_KHR_swapchain",
		};
	}

	STL::Vector<const char*> Platform::GetRequestedVulkanDeviceLayers()
	{
		return {};
	}

	void Platform::WriteToConsole(const char* message, const EConsoleTextColor color)
	{
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
}
#endif