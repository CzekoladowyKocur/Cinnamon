#pragma once
#define InternalScope static
#define FunctionVariable static

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

#ifdef CIN_PLATFORM_WINDOWS
#define CIN_DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define CIN_DEBUG_BREAK() raise(SIGABRT)
#endif

#ifdef CIN_DEBUG
#define CIN_EXPAND_VARGS(x) x
#define CIN_ASSERT_CONDITION(statement) { if(!(statement)) { CIN_DEBUG_BREAK(); } }
#define CIN_ASSERT_MESSAGE(statement, ...) { if(!(statement)) { CIN_DEBUG_BREAK(); } }

#define CIN_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
#define CIN_GET_ASSERT_MACRO(...) CIN_EXPAND_VARGS(CIN_ASSERT_RESOLVE(__VA_ARGS__, CIN_ASSERT_MESSAGE, CIN_ASSERT_CONDITION))

#define CIN_ASSERT(...) CIN_EXPAND_VARGS( CIN_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define CIN_CORE_ASSERT(...) CIN_EXPAND_VARGS( CIN_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
#define CIN_ASSERT(...)
#define CIN_CORE_ASSERT(...)
#endif
#define CIN_UNIMPLEMENTED() CIN_ASSERT(false, "Unimplemeneted")
#define CIN_UNUSED(x) (void)x

consteval auto ResolveAtCompileTime(auto arg)
{
	return arg;
}
