// $(header)
// Printf-style interface for display OS message boxes
#pragma once
#ifndef oMsgBox_h
#define oMsgBox_h

#include <stdarg.h>

struct oMsgBox
{
	enum TYPE
	{
		INFO,
		WARN,
		YESNO,
		ERR,
		DEBUG,
	};

	enum RESULT
	{
		NO,
		YES, // any "OK" button returns YES
		ABORT,
		BREAK,
		CONTINUE,
		IGNORE_ALWAYS,
	};

	static RESULT tvprintf(TYPE _Type, unsigned int _Timeout, const char* _Title, const char* _Format, va_list _Args);
	static RESULT tprintf(TYPE _Type, unsigned int _Timeout, const char* _Title, const char* _Format, ...);
	static RESULT vprintf(TYPE _Type, const char* _Title, const char* _Format, va_list _Args);
	static RESULT printf(TYPE _Type, const char* _Title, const char* _Format, ...);
};

#endif
