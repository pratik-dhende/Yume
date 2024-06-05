#pragma once

#ifdef YM_PLATFORM_WINDOWS
	#ifdef YM_BUILD_DLL
		#define YM_API __declspec(dllexport)
	#else
		#define	YM_API __declspec(dllimport)
	#endif

	// Constants
	#include<string>
	const std::wstring YM_DLL_FILE_NAME = L"Yume.dll";
	const std::wstring YM_ENGINE_NAME = L"Yume";
#else
	#error Yume only supports Windows!
#endif