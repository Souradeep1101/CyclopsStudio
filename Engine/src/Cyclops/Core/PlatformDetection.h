#pragma once

// Windows
#ifdef _WIN32
	/* _WIN32 is defined for both x86 and x64.
	   _WIN64 is defined ONLY for x64. */
	#ifdef _WIN64
		#define CYCLOPS_PLATFORM_WINDOWS
	#else
		#error "x86 Builds are not supported!"
	#endif

// Apple (MacOS / iOS)
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all Apple platforms
	   TARGET_OS_IPHONE exists only on iOS/embedded */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS Simulator not supported!"
	#elif TARGET_OS_IPHONE == 1
		#error "IOS not supported!"
	#elif TARGET_OS_MAC == 1
		#define CYCLOPS_PLATFORM_MAC
	#else
		#error "Unknown Apple Platform!"
	#endif

// Linux
#elif defined(__linux__)
	#define CYCLOPS_PLATFORM_LINUX

#else
	#error "Unknown Platform!"
#endif
