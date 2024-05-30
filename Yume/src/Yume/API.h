#pragma once

#ifdef YM_PLATFORM_WINDOWS
	#ifdef YM_BUILD_DLL
		#define YM_API __declspec(dllexport)
	#else
		#define	YM_API __declspec(dllimport)
	#endif
#else
	#error Yume only supports Windows!
#endif