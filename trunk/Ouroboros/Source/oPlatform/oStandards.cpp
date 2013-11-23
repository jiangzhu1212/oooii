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
#include <oGUI/console.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/oMsgBox.h>
#include <oPlatform/oStream.h>

using namespace ouro;

void oConsoleReporting::VReport( REPORT_TYPE _Type, const char* _Format, va_list _Args )
{
	static const color fg[] = 
	{
		color(0),
		Lime,
		White,
		color(0),
		Yellow,
		Red,
		Yellow,
	};
	static_assert(oCOUNTOF(fg) == NUM_REPORT_TYPES, "");

	static const color bg[] = 
	{
		color(0),
		color(0),
		color(0),
		color(0),
		color(0),
		color(0),
		Red,
	};
	static_assert(oCOUNTOF(fg) == NUM_REPORT_TYPES, "");

	if (_Type == HEADING)
	{
		char msg[2048];
		vsnprintf(msg, _Format, _Args);
		toupper(msg);
		ouro::console::fprintf(stdout, fg[_Type], bg[_Type], msg);
	}
	else
	{
		ouro::console::vfprintf(stdout,fg[_Type], bg[_Type], _Format, _Args );
	}
}

bool oMoveMouseCursorOffscreen()
{
	int2 p, sz;
	ouro::display::virtual_rect(&p.x, &p.y, &sz.x, &sz.y);
	return !!SetCursorPos(p.x + sz.x, p.y-1);
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