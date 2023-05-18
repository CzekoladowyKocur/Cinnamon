    #pragma once
#define InternalScope static
#define FunctionVariable static

#ifdef __DATE__
#define CIN_DATE __DATE__
#else
#error Define me!
#endif

#ifdef __TIME__
#define CIN_TIME __TIME__
#else
#error Define me!
#endif

#ifdef __TIMESTAMP__
#define CIN_TIMESTAMP __TIMESTAMP__
#else
#error Define me!
#endif

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

#ifdef CIN_PLATFORM_WINDOWS
#define CIN_DISABLE_WARNINGS pragma warning(push, 0)
#define CIN_ENABLE_WARNINGS pragma warning(pop)
#elif defined CIN_PLATFORM_LINUX
#define CIN_DISABLE_WARNINGS 
#define CIN_ENABLE_WARNINGS 
/* Todo */
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
#define CIN_ASSERT_MESSAGE(statement, ...) { if(!(statement)) { printf(__VA_ARGS__); CIN_DEBUG_BREAK(); } }

#define CIN_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
#define CIN_GET_ASSERT_MACRO(...) CIN_EXPAND_VARGS(CIN_ASSERT_RESOLVE(__VA_ARGS__, CIN_ASSERT_MESSAGE, CIN_ASSERT_CONDITION))

#define CIN_ASSERT(...) CIN_EXPAND_VARGS( CIN_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define CIN_CORE_ASSERT(...) CIN_EXPAND_VARGS( CIN_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define CIN_VERIFY(x) if(!(x)) CIN_ASSERT(false);
#else
#define CIN_ASSERT(...)
#define CIN_CORE_ASSERT(...)
#define CIN_VERIFY(x) { if(!(x)) { std::exit(EXIT_FAILURE); } }
#endif
#define CIN_UNIMPLEMENTED() CIN_ASSERT(false, "Unimplemeneted")
#define CIN_UNUSED(x) (void)x

#define CIN_PANIC_EXIT() exit(EXIT_FAILURE)

#if defined(__clang__) || defined(__GNUC__)
#define CIN_STATIC_ASSERT	_Static_assert
#define CIN_FORCE_INLINE	inline __attribute__((always_inline))
#else
#define CIN_STATIC_ASSERT	static_assert
#define CIN_FORCE_INLINE 	__forceinline
#endif

/* Math*/
#define CIN_CLAMP(value, min, max) ((value) < (min)) ? (min) : ((value) > (max)) ? (max) : (value); 
#define CIN_MAX(value, max) ((value) < (max)) ? (value) : (max);
#define CIN_MIN(value, min) ((value) > (min)) ? (value) : (min);
#define CIN_MIN_MAX(value, min, max) CIN_CLAMP(value, min, max)

#define CIN_CARRAY_SIZE(array) (uint32_t)((sizeof(array) / sizeof(*(array))))

#define NON_CONSTRUCTIBLE(classType)                            \
	constexpr explicit classType() noexcept = delete;           \
	constexpr ~classType() noexcept = delete;

#define NON_COPYABLE(classType)                                 \
    classType(const classType&) noexcept = delete;              \
    classType& operator=(const classType&) noexcept  = delete; 

#define NON_MOVABLE(classType)                                  \
    classType(classType&&) noexcept = delete;                   \
    classType& operator=(classType&&) noexcept = delete;

consteval auto ResolveAtCompileTime(auto arg)
{
	return arg;
}

#ifdef __unix
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))
#endif

class Timestep final
{
public:
    using Type = float;

    constexpr explicit Timestep() noexcept;
    explicit Timestep(const Type deltaTime) noexcept;
    constexpr ~Timestep() noexcept = default;

    Type GetTimestep() const;
    operator Type() const;
private:
    Type m_DeltaTime;
};

enum class ErrorCode
{
    Failure = 0U,
    Success
};

struct [[nodiscard]] Errr final
{
    ErrorCode Error;

	consteval explicit Errr(const ErrorCode error) noexcept
        :
        Error(error)
    {}

    constexpr operator bool() noexcept
    {
        return Error == ErrorCode::Success;
    }
};

namespace Error {
    static constexpr Errr Success{ ErrorCode::Success };
    static constexpr Errr Failure{ ErrorCode::Failure };
}