// $(header)
// Printf-style interface for displaying OS message boxes
#pragma once
#ifndef oMsgBox_h
#define oMsgBox_h

#include <oPlatform/oStddef.h>
#include <stdarg.h>

enum oMSGBOX_TYPE
{
	oMSGBOX_INFO,
	oMSGBOX_WARN,
	oMSGBOX_YESNO,
	oMSGBOX_ERR,
	oMSGBOX_DEBUG,
	oMSGBOX_NOTIFY,
	oMSGBOX_NOTIFY_INFO,
	oMSGBOX_NOTIFY_WARN,
	oMSGBOX_NOTIFY_ERR,
};

enum oMSGBOX_RESULT
{
	oMSGBOX_NO,
	oMSGBOX_YES,
	oMSGBOX_ABORT,
	oMSGBOX_BREAK,
	oMSGBOX_CONTINUE,
	oMSGBOX_IGNORE,
};

struct oMSGBOX_DESC
{
	oMSGBOX_DESC()
		: Type(oMSGBOX_INFO)
		, TimeoutMS(oINFINITE_WAIT)
		, ParentNativeHandle(nullptr)
		, Title("")
	{}

	oMSGBOX_TYPE Type;
	unsigned int TimeoutMS;
	void* ParentNativeHandle;
	const char* Title;
};


// For no timeout, specify oINFINITE_WAIT
oMSGBOX_RESULT oMsgBoxV(const oMSGBOX_DESC& _Desc, const char* _Format, va_list _Args);
inline oMSGBOX_RESULT oMsgBox(const oMSGBOX_DESC& _Desc, const char* _Format, ...) { va_list args; va_start(args, _Format); return oMsgBoxV(_Desc, _Format, args); }

#endif
