#pragma once
/* Standard includes */
#include <stdint.h>
#include <utility>
#include <stddef.h>
#include <cstddef>
#include <cstdlib>
#include <string_view>
#include <limits>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <functional>
#include <chrono>
#include <array>
#include <string>
#include <cstring>
#include <string_view>
#include <type_traits>
#include <queue>

#ifdef CIN_PLATFORM_WINDOWS
#undef UNICODE
#ifndef UNICODE
#define _T(x) x
#endif
#include <Windows.h>
#include <Windowsx.h>
#include <shellapi.h>
#undef max
#undef min
#elif defined CIN_PLATFORM_LINUX
#endif

/* Platform */
#include "Platform/Platform.h"
/* Engine includes */
#include "Cinnamon/include/Core/TypeDefines.h"
#include "Cinnamon/include/Core/Logger.h"
#include "Cinnamon/include/Core/CinSTL.h"
#include "Cinnamon/include/Memory/CinMemory.h"
#include "Cinnamon/include/Memory/Allocator.h"