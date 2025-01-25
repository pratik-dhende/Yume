#pragma once

#ifdef YM_PLATFORM_WINDOWS
	#include "Utility/Utility.h"

	#ifdef YM_ENABLE_ASSERTS
		#define YM_CORE_ASSERT(x,...) { if(!(x)) { YM_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
		#define YM_ASSERT(x,...) { if(!(x)) { YM_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#else
		#define YM_CORE_ASSERT(x,...)
		#define YM_ASSERT(x,...)
	#endif

#ifdef YM_DEBUG
	#define YM_MAIN() main(int argc, char** argv)
	#define YM_N_CMD_SHOW SW_SHOW
#else
	#define YM_MAIN() WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
	#define YM_N_CMD_SHOW nCmdShow
#endif

	// Constants
	#include<string>
	const std::wstring YM_DLL_FILE_NAME = L"Yume.dll";
	const std::wstring YM_ENGINE_NAME = L"Yume";
#else
	#error Yume only supports Windows!
#endif