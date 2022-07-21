#pragma once
/* Standard includes */
#include <stdint.h>
#include <utility>
#include <stddef.h>
#include <cstdlib>
/* Engine includes */
#include "Cinnamon/include/Core/TypeDefines.h"
#include "Cinnamon/include/Memory/CinMemory.h"

#ifdef __FILE__ 
#define CIN_FILE __FILE__
#else
#error Define me!
#endif

#ifdef __LINE__ 
#define CIN_LINE __LINE__ 
#else
#error Define me!
#endif

#define BIT(n) 1 << (n - 1)
