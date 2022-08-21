#pragma once

#include "Cinnamon/include/Core/Application.h"

#ifndef EXIT_SUCCESS 
#define EXIT_SUCCESS 0
#endif // !EXIT_SUCCESS 

#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif // !EXIT_FAILURE 

InternalScope int32_t CommonEntryPoint() noexcept
{
	using namespace Cinnamon;
	/* Initialize platform */
	if (!Platform::Initialize())
	{
		printf("Failed to initialize platform\n");
		return EXIT_FAILURE;
	}

	if (!Logger::Initialize(ELogLevel::Trace))
	{
		printf("Failed to initialize platform\n");
		return EXIT_FAILURE;
	}

	/* Application lifetime */
	{
		Application* application{ CreateApplication() };
		if (!application->Initialize())
		{
			CIN_CRITICAL("Failed to properly initialize the application");
			return EXIT_FAILURE;
		}

		if (!application->Run())
		{
			CIN_CRITICAL("Failed to properly run the application");
			return EXIT_FAILURE;
		}

		if (!application->Shutdown())
		{
			CIN_CRITICAL("Failed to properly shutdown the application");
			return EXIT_FAILURE;
		}

		cindel application;
	} /* Application lifetime */

	/* Shutdown platform */
	if (!Logger::Shutdown())
	{
		CIN_CRITICAL("Failed to shutdown logger");
		return EXIT_FAILURE;
	}

	if (!Platform::Shutdown())
	{
		CIN_CRITICAL("Failed to shutdown platform");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifdef CIN_PLATFORM_WINDOWS
#ifdef APIENTRY
#undef APIENTRY
#endif /* APIENTRY */
#define USE_CRT_MEMORY_LEAK_DETECTION 0 /* 1 */
/* CRT detection tracks malloc only, so we override the new operators */
#if USE_CRT_MEMORY_LEAK_DETECTION
void* operator new(const std::size_t size)
{
	return malloc(size);
}

void* operator new[](const std::size_t size)
{
	return malloc(size);
}
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif 

static_assert(sizeof(std::remove_pointer<LPWSTR>::type) == sizeof(char16_t), "UTF-16 Encoding for windows required");
INT WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd)
{
	CIN_UNUSED(hInstance);
	CIN_UNUSED(hPrevInstance);
	CIN_UNUSED(lpCmdLine);
	CIN_UNUSED(nShowCmd);

#if USE_CRT_MEMORY_LEAK_DETECTION
	/* Using memory checkpoint to prevent static object initialization and deinitialization */
	_CrtMemState memoryState;
	_CrtMemCheckpoint(&memoryState);
#endif
	/// https://jdelezenne.github.io/Codex/
	INT argc{ 0 };
	LPWSTR* argv{ CommandLineToArgvW(GetCommandLineW(), &argc) };
	(void)argv;
	(void)argc;
	int result{ EXIT_SUCCESS };
	{
		result = CommonEntryPoint(/*std::move(arguments)*/);
	}
#if USE_CRT_MEMORY_LEAK_DETECTION
	/* No point to log in case of invalid application initialization */
	if (result == EXIT_SUCCESS)
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);

		_CrtMemCheckpoint(&memoryState);
	}
#endif
	return result;
} /* Windows */
#elif defined CIN_PLATFORM_LINUX
/* UTF8 only */
int main(const int argc, const char** argv)
{
	//CommandLineArguments arguments(static_cast<const char>(argc), (const char**)argv);
    (void)argc;
    (void)argv;

	return CommonEntryPoint();
} /* Linux */
#elif defined CIN_PLATFORM_APPLE
/* UTF8 only */
int main(const char argc, const char** argv)
{
	CommandLineArguments arguments(static_cast<const char>(argc), (const char**)argv);
	return CommonEntryPoint(arguments);
} /* MacOS */
#elif defined CIN_PLATFORM_IOS
/* UTF8 only */
int main(const char argc, const char** argv)
{
	CommandLineArguments arguments(static_cast<const char>(argc), (const char**)argv);
	return CommonEntryPoint(arguments);
} /* IOS */
#elif defined CIN_PLATFORM_ANDROID
/* UTF8 only */
int main(const char argc, const char** argv)
{
	CommandLineArguments arguments(static_cast<const char>(argc), (const char**)argv);
	return CommonEntryPoint(arguments);
} /* Android */
#endif