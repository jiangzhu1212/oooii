// $(header)

// Interface for working with an attached command line display
#pragma once
#ifndef oConsole_h
#define oConsole_h

#include <oBasis/oColor.h>
#include <oBasis/oFunction.h>
#include <oBasis/oMathTypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace oConsole
{
	const unsigned short DEFAULT = 0x8000;

	// Return true to short-circuit any default action. Return false to defer
	// to default behavior.
	typedef oFUNCTION<bool()> EventFn;

	enum EVENT
	{
		CTRLC,
		CTRLBREAK,
		CLOSE, // you've got 5-sec on Windows before it force-closes
		LOGOFF,
		SHUTDOWN,
	};
	
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
	int2 GetSizeInPixels();
	int2 GetSizeInCharacters();

	void SetDesc(const DESC* _pDesc);
	void GetDesc(DESC* _pDesc);
	void SetTitle(const char* _Title);
	void GetTitle(char* _StrDestination, size_t _SizeofStrDestination);
	template<size_t size> inline void GetTitle(char (&_StrDestination)[size]) { return GetTitle(_StrDestination, size); }

	// If you specify DEFAULT for one of these, then it will be whatever it was
	// before.
	void SetCursorPosition(const int2& _Position);
	int2 GetCursorPosition();

	void Clear();

	bool HasFocus();

	int vfprintf(FILE* _pStream, oColor _Foreground, oColor _Background, const char* _Format, va_list _Args);
	inline int fprintf(FILE* _pStream, oColor _Foreground, oColor _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); return vfprintf(_pStream, _Foreground, _Background, _Format, args); }
	inline int printf(oColor _Foreground, oColor _Background, const char* _Format, ...) { va_list args; va_start(args, _Format); return vfprintf(stdout, _Foreground, _Background, _Format, args); }

	void HookEvent(EVENT _Event, EventFn _Function);
};

#endif
