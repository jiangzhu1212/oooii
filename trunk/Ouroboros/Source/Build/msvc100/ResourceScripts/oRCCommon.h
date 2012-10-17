// $(header)
// This file is intended to be included by .rc MS Visual Studio resource 
// scripts, NOT C++. It contains generic or Windows-related definitions 
// that should be applicable to any project. Note that certain per-project
// defines are required.
#pragma once
#ifndef oRCCommon_h
#define oRCCommon_h

// _____________________________________________________________________________
// Checks to ensure external defines are defined

#ifndef oRCCopyrightString
	#error undefined oRCCopyrightString
#endif
#ifndef oRCCompanyName
	#error undefined oRCCompanyName
#endif
#ifndef oRCProductName
	#error undefined oRCProductName
#endif

#ifdef oRCIsSpecialBuild
	#ifndef oRCSpecialBuildMessage
		#error undefined oRCSpecialBuildMessage
	#endif
#endif

// _____________________________________________________________________________
// Defines for the flags used by Windows

// Copy-pasted from VerRsrc.h
/* ----- VS_VERSION.dwFileFlags ----- */
#define VS_FF_DEBUG             0x00000001L
#define VS_FF_PRERELEASE        0x00000002L
#define VS_FF_PATCHED           0x00000004L
#define VS_FF_PRIVATEBUILD      0x00000008L
#define VS_FF_INFOINFERRED      0x00000010L
#define VS_FF_SPECIALBUILD      0x00000020L

/* ----- VS_VERSION.dwFileOS ----- */
#define VOS_UNKNOWN             0x00000000L
#define VOS_DOS                 0x00010000L
#define VOS_OS216               0x00020000L
#define VOS_OS232               0x00030000L
#define VOS_NT                  0x00040000L
#define VOS_WINCE               0x00050000L

#define VOS__BASE               0x00000000L
#define VOS__WINDOWS16          0x00000001L
#define VOS__PM16               0x00000002L
#define VOS__PM32               0x00000003L
#define VOS__WINDOWS32          0x00000004L

#define VOS_DOS_WINDOWS16       0x00010001L
#define VOS_DOS_WINDOWS32       0x00010004L
#define VOS_OS216_PM16          0x00020002L
#define VOS_OS232_PM32          0x00030003L
#define VOS_NT_WINDOWS32        0x00040004L

/* ----- VS_VERSION.dwFileType ----- */
#define VFT_UNKNOWN             0x00000000L
#define VFT_APP                 0x00000001L
#define VFT_DLL                 0x00000002L
#define VFT_DRV                 0x00000003L
#define VFT_FONT                0x00000004L
#define VFT_VXD                 0x00000005L
#define VFT_STATIC_LIB          0x00000007L

/* ----- VS_VERSION.dwFileSubtype for VFT_WINDOWS_DRV ----- */
#define VFT2_UNKNOWN            0x00000000L
#define VFT2_DRV_PRINTER        0x00000001L
#define VFT2_DRV_KEYBOARD       0x00000002L
#define VFT2_DRV_LANGUAGE       0x00000003L
#define VFT2_DRV_DISPLAY        0x00000004L
#define VFT2_DRV_MOUSE          0x00000005L
#define VFT2_DRV_NETWORK        0x00000006L
#define VFT2_DRV_SYSTEM         0x00000007L
#define VFT2_DRV_INSTALLABLE    0x00000008L
#define VFT2_DRV_SOUND          0x00000009L
#define VFT2_DRV_COMM           0x0000000AL
#define VFT2_DRV_INPUTMETHOD    0x0000000BL
#define VFT2_DRV_VERSIONED_PRINTER    0x0000000CL

/* ----- VS_VERSION.dwFileSubtype for VFT_WINDOWS_FONT ----- */
#define VFT2_FONT_RASTER        0x00000001L
#define VFT2_FONT_VECTOR        0x00000002L
#define VFT2_FONT_TRUETYPE      0x00000003L

// _____________________________________________________________________________
// Respect OOOii common build flags. Ensure these are defined for the resource
// compiler, not just the C++ compiler.

#ifdef _DLL
	#ifdef oSTATICLIB
		#error oSTATICLIB and _DLL defined at same time
	#endif
	#define oRCFileType VFT_DLL
	#define oRCModuleSuffix ".dll"

#elif defined(oSTATICLIB)
	#ifdef _DLL
		#error _DLL and oSTATICLIB defined at same time
	#endif
	#define oRCFileType VFT_STATIC_LIB
	#define oRCModuleSuffix ".lib"

#else
	#define oRCFileType VFT_APP
	#define oRCModuleSuffix ".exe"

#endif

#ifdef oRCIsSpecialBuild
	#define oRCVersionSuffix " (Special)"
#else
	#define oRCVersionSuffix ""
#endif

#ifdef _DEBUG
	#define oRCBuildPrefix "DEBUG-"
	#define oRCBuildSuffix " (Debug)"

	#ifdef oRCIsSpecialBuild
		#define oRCFileFlags VS_FF_DEBUG|VS_FF_SPECIALBUILD
	#else
		#define oRCFileFlags VS_FF_DEBUG
	#endif
#else
	#define oRCBuildPrefix
	#define oRCBuildSuffix
	#ifdef oRCIsSpecialBuild
		#define oRCFileFlags VS_FF_SPECIALBUILD
	#else
		#define oRCFileFlags 0
	#endif
#endif

#endif
