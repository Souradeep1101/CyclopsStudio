#pragma once

// 1. Detect the Platform FIRST
#include "PlatformDetection.h" 

// 2. Then define the API macro based on the platform
#ifdef CYCLOPS_PLATFORM_WINDOWS
	#ifdef CYCLOPS_BUILD_DLL
		#define CYCLOPS_API __declspec(dllexport)
	#else
		#define CYCLOPS_API __declspec(dllimport)
	#endif
#else
	#define CYCLOPS_API
#endif