/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#pragma once
#ifndef oCore_win_version_h
#define oCore_win_version_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ouro {
	namespace windows {

/* enum class */ namespace version
{	enum value {

	unknown,
	win2000,
	xp,
	xp_pro_64bit,
	server_2003,
	home_server,
	server_2003r2,
	vista,
	server_2008,
	vista_sp1,
	server_2008_sp1,
	vista_sp2,
	server_2008_sp2,
	win7,
	server_2008r2,
	win7_sp1,
	server_2008r2_sp1,
	win8,
	server_2012,
	win8_1,
	server_2012_sp1,

};}

version::value get_version();

	} // namespace windows
} // namespace ouro

#endif
