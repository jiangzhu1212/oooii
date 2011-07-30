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
#include <oooii/oooii.h>
#include <oooii/oStandards.h>

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
	oSTATICASSERT(oCOUNTOF(fg) == NUM_REPORT_TYPES);

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
	oSTATICASSERT(oCOUNTOF(fg) == NUM_REPORT_TYPES);

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
