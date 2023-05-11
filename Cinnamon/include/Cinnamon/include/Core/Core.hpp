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
#include <map>
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
#include <filesystem>
#include <set>
#include <future>
#include <algorithm>
#include <condition_variable>
#include <fstream>

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
#include "Platform/Platform.hpp"
/* Engine includes */
#include "Cinnamon/include/Core/TypeDefines.hpp"
#include "Cinnamon/include/Core/Logger.hpp"
#include "Cinnamon/include/Core/CinSTL.hpp"
#include "Cinnamon/include/Memory/CinMemory.hpp"
#include "Cinnamon/include/Memory/Allocator.hpp"