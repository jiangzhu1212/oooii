/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oBase/assert.h>
#include <oStd/chrono.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/oStream.h>

using namespace ouro;

void oConsoleReporting::VReport( REPORT_TYPE _Type, const char* _Format, va_list _Args )
{
	static const color fg[] = 
	{
		0,
		Lime,
		White,
		0,
		Yellow,
		Red,
		Yellow,
	};
	static_assert(oCOUNTOF(fg) == NUM_REPORT_TYPES, "");

	static const color bg[] = 
	{
		0,
		0,
		0,
		0,
		0,
		0,
		Red,
	};
	static_assert(oCOUNTOF(fg) == NUM_REPORT_TYPES, "");

	if (_Type == HEADING)
	{
		char msg[2048];
		vsnprintf(msg, _Format, _Args);
		toupper(msg);
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
	ouro::display::virtual_rect(&p.x, &p.y, &sz.x, &sz.y);
	return !!SetCursorPos(p.x + sz.x, p.y-1);
}

bool oWaitForSystemSteadyState(oFUNCTION<bool()> _ContinueIdling)
{
	//if (oProcessHasDebuggerAttached(oProcessGetCurrentID()))
	//		return true;

	oTRACE("Waiting for system steady state...");

	const unsigned int TWO_MINUTES = 120000;
	if (!ouro::system::wait_for_idle(TWO_MINUTES, _ContinueIdling))
	{
		if (oErrorGetLast() == ETIMEDOUT)
		{
			oMSGBOX_DESC d;
			d.Type = oMSGBOX_ERR;
			d.Title = "ExProx2.exe";
			d.TimeoutMS = 30000;
			oMsgBox(d, "The application waited for the system to reach a steady state, but it did not occur within the timeout period.");
			return false;
		}

		else if (oErrorGetLast() != std::errc::operation_canceled)
		{
			oMSGBOX_DESC d;
			d.Type = oMSGBOX_ERR;
			d.Title = "ExProx2.exe";
			d.TimeoutMS = 30000;
			oMsgBox(d, "The application waited for the system to reach a steady state, but it did not occur for a reason other than timeout (needs more debugging).");
			return false;
		}
	}

	oTRACE("System has steadied, continuing...");
	return true;
}

void* oLoadIcon(oFUNCTION<void(const char** _ppBufferName, const void** _ppBuffer, size_t* _pSizeofBuffer)> _BufferGetDesc)
{
	const char* BufferName = nullptr;
	const void* pBuffer = nullptr;
	size_t sizeofBuffer = 0;
	_BufferGetDesc(&BufferName, &pBuffer, &sizeofBuffer);

	intrusive_ptr<oImage> ico;
	oVERIFY(oImageCreate(BufferName, pBuffer, sizeofBuffer, &ico));

	#if defined(_WIN32) || defined (_WIN64)
		oGDIScopedObject<HBITMAP> hBmp;
		oVERIFY(oImageCreateBitmap(ico, (HBITMAP*)&hBmp));
		return oGDIBitmapToIcon(hBmp);
	#else
		return nullptr;
	#endif
}

void* oLoadInvisibleIcon()
{
	extern void GetDescoInvisible_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
	return oLoadIcon(GetDescoInvisible_ico);
}

void* oLoadStandardIcon()
{
	extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
	return oLoadIcon(GetDescoooii_ico);
}

char* oGetLogFilePath(char* _StrDestination, size_t _SizeofStrDestination, const char* _ExeSuffix)
{
	time_t theTime;
	time(&theTime);
	tm t;
	localtime_s(&t, &theTime);

	path AppPath = ouro::filesystem::app_path(true);
	if (strlcpy(_StrDestination, AppPath, _SizeofStrDestination) >= _SizeofStrDestination)
		return nullptr; // pass through error

	char fn[128];
	char* p = oGetFilebase(_StrDestination);
	strlcpy(fn, p);
	oTrimFileExtension(fn);
	p += snprintf(p, _SizeofStrDestination - std::distance(_StrDestination, p), "Logs/");
	p += strftime(p, _SizeofStrDestination - std::distance(_StrDestination, p), "%Y-%m-%d-%H-%M-%S_", &t);
	p += snprintf(p, _SizeofStrDestination - std::distance(_StrDestination, p), "%s", fn);

	errno_t err = 0;
	if (_ExeSuffix)
		p += snprintf(p, _SizeofStrDestination - std::distance(_StrDestination, p), "_%s", _ExeSuffix);

	p += snprintf(p, _SizeofStrDestination - std::distance(_StrDestination, p), "_%i.txt", ouro::this_process::get_id());
	clean_path(_StrDestination, _SizeofStrDestination, _StrDestination);
	
	return _StrDestination;
}

bool oINIFindPath( char* _StrDestination, size_t _SizeofStrDestination, const char* _pININame )
{
	snprintf(_StrDestination, _SizeofStrDestination, "../%s", _pININame);
	if(oStreamExists(_StrDestination))
		return true;

	path AppDir = ouro::filesystem::app_path();

	snprintf(_StrDestination, _SizeofStrDestination, "%s/../%s", AppDir, _pININame);
	if(oStreamExists(_StrDestination))
		return true;

	snprintf(_StrDestination, _SizeofStrDestination, "%s", _pININame);
	if(oStreamExists(_StrDestination))
		return true;

	snprintf(_StrDestination, _SizeofStrDestination, "%s/%s", AppDir, _pININame);
	if(oStreamExists(_StrDestination))
		return true;

	return oErrorSetLast(std::errc::no_such_file_or_directory, "No ini file %s found.", _pININame);
}