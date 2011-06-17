// $(header)

// Interface for working with an attached command line display
#pragma once
#ifndef oConsole_h
#define oConsole_h

#include <oooii/oColor.h>
#include <stdarg.h>
#include <stdio.h>

namespace oConsole
{
	const unsigned short DEFAULT = 0x8000;

	struct DESC
	{
		DESC()
			: BufferWidth(DEFAULT)
			, BufferHeight(DEFAULT)
			, Top(DEFAULT)
			, Left(DEFAULT)
			, Width(DEFAULT)
			, Height(DEFAULT)
			, Foreground(0)
			, Background(0)
			, Show(true)
		{}

		// NOTE: BufferWidth/Height must be the same size or larger than Width/Height

		unsigned short BufferWidth; // in characters
		unsigned short BufferHeight;  // in characters
		unsigned short Top; // in pixels
		unsigned short Left;  // in pixels
		unsigned short Width;  // in characters
		unsigned short Height;  // in characters
		oColor Foreground; // 0 means don't set color
		oColor Background; // 0 means don't set color
		bool Show;
	};

	void* GetNativeHandle(); // returns HWND on Windows

	// Get the pixel width/height of the console window
	void GetPixelWidthHeight(unsigned int* _pPixelWidth, unsigned int* _pPixelHeight);

	void SetDesc(const DESC* _pDesc);
	void GetDesc(DESC* _pDesc);
	void SetTitle(const char* _Title);
	void GetTitle(char* _StrDestination, size_t _SizeofStrDestination);
	template<size_t size> inline void GetTitle(char (&_StrDestination)[size]) { return GetTitle(_StrDestination, size); }

	// If you specify DEFAULT for one of these, then it will be whatever it was
	// before.
	void SetCursorPosition(unsigned short _X, unsigned short _Y);
	void GetCursorPosition(unsigned short* _pX, unsigned short* _pY);

	bool HasFocus();

	int vfprintf(FILE* _pStream, oColor _Foreground, oColor _Background, const char* _Format, va_list _Args);
	inline int fprintf(FILE* _pStream, oColor _Foreground, oColor _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); return vfprintf(_pStream, _Foreground, _Background, _Format, args); }
	inline int printf(oColor _Foreground, oColor _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); return vfprintf(stdout, _Foreground, _Background, _Format, args); }
};

#endif
