// $(header)

// See oMutex.h for why this is exposed. This header must forward-declare -
// without including any platform headers - the platform space for the class.
// This is necessary to ensure that a mutex remains fast by avoiding a 
// virtual interface and still remain somewhat abstract.

// The goal is to define the macros oMUTEX_FOOTPRINT() and oRWMUTEX_FOOTPRINT()
// for use in oMutex.h
#pragma once
#ifndef oMutexInternal_h
#define oMutexInternal_h

#if defined(_WIN32) || defined(_WIN64)

	// oMUTEX_FOOTPRINT
	#ifdef _WIN64
		#define oMUTEX_FOOTPRINT() mutable unsigned long long Footprint[5] // RTL_CRITICAL_SECTION
	#elif defined(_WIN32)
		#define oMUTEX_FOOTPRINT() mutable unsigned int Footprint[6]
	#endif

	// oRWMUTEX_FOOTPRINT
	#ifdef _DEBUG
		// Debug info for tagging which thread we're on to allow for protection against
		// attempts to recursively lock.
		#define oRWMUTEX_FOOTPRINT() void* Footprint; mutable size_t ThreadID
	#else
		#define oRWMUTEX_FOOTPRINT() void* Footprint
	#endif

#else
	#error Unsupported platform (oMUTEX_FOOTPRINT, oRWMUTEX_FOOTPRINT)
#endif
#endif
