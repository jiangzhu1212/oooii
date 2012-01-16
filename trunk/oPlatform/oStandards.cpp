/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oPlatform/oStandards.h>
#include <oBasis/oAssert.h>
#include <oBasis/oRef.h>
#include <oBasis/oStdChrono.h>
#include <oBasis/oString.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oGDI.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oWindows.h>

int oWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow, int (*oMain)(int argc, const char* argv[]))
{
	int argc = 0;
	const char** argv = oWinCommandLineToArgvA(true, lpCmdLine, &argc);
	int result = oMain(argc, argv);
	oWinCommandLineToArgvAFree(argv);
	return result;
}

void oConsoleReporting::VReport( REPORT_TYPE _Type, const char* _Format, va_list _Args )
{
	static const oColor fg[] = 
	{
		0,
		std::Lime,
		std::White,
		0,
		std::Yellow,
		std::Red,
		std::Yellow,
	};
	static_assert(oCOUNTOF(fg) == NUM_REPORT_TYPES, "");

	static const oColor bg[] = 
	{
		0,
		0,
		0,
		0,
		0,
		0,
		std::Red,
	};
	static_assert(oCOUNTOF(fg) == NUM_REPORT_TYPES, "");

	if (_Type == HEADING)
	{
		char msg[2048];
		vsprintf_s(msg, _Format, _Args);
		oToUpper(msg);
		oConsole::fprintf(stdout, fg[_Type], bg[_Type], msg);
	}
	else
	{
		oConsole::vfprintf(stdout,fg[_Type], bg[_Type], _Format, _Args );
	}
}

bool oMoveMouseCursorOffscreen()
{
	int2 p, sz;
	oDisplayGetVirtualRect(&p, &sz);
	return !!SetCursorPos(p.x + sz.x, p.y-1);
}
void* oLoadStandardIcon()
{
	// Load/test OOOii lib icon:
	extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
	const char* BufferName = nullptr;
	const void* pBuffer = nullptr;
	size_t bufferSize = 0;
	GetDescoooii_ico(&BufferName, &pBuffer, &bufferSize);

	oRef<oImage> ico;
	oVERIFY(oImageCreate("Icon image", pBuffer, bufferSize, &ico));

	#if defined(_WIN32) || defined (_WIN64)
		oGDIScopedObject<HBITMAP> hBmp;
		oVERIFY(oImageCreateBitmap(ico, (HBITMAP*)&hBmp));
		return oIconFromBitmap(hBmp);
	#else
		return nullptr;
	#endif
}

errno_t oGetLogFilePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _ExeSuffix)
{
	time_t theTime;
	time(&theTime);
	tm t;
	localtime_s(&t, &theTime);

	if (!oSystemGetPath(_StrDestination, _SizeofStrDestination, oSYSPATH_APP_FULL))
		return oErrorGetLast();

	char fn[128];
	char* p = oGetFilebase(_StrDestination);
	strcpy_s(fn, p);
	oTrimFileExtension(fn);
	p += sprintf_s(p, _SizeofStrDestination - std::distance(_StrDestination, p), "Logs/");
	p += strftime(p, _SizeofStrDestination - std::distance(_StrDestination, p), "%Y-%m-%d-%H-%M-%S_", &t);
	p += sprintf_s(p, _SizeofStrDestination - std::distance(_StrDestination, p), "%s", fn);

	errno_t err = 0;
	if (_ExeSuffix)
		p += sprintf_s(p, _SizeofStrDestination - std::distance(_StrDestination, p), "_%s", _ExeSuffix);

	p += sprintf_s(p, _SizeofStrDestination - std::distance(_StrDestination, p), "_%i.txt", oProcessGetCurrentID());
	oCleanPath(_StrDestination, _SizeofStrDestination, _StrDestination);
	return 0;
}

