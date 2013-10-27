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
// Interface for working with monitors/displays
#pragma once
#ifndef oCore_display_h
#define oCore_display_h

#include <oBase/macros.h>

namespace ouro {
	namespace display {

class id
{
public:
	id() : Handle(-1) {}

	bool operator==(const id& _That) const { return Handle == _That.Handle; }
	bool operator!=(const id& _That) const { return !(*this == _That); }
	operator bool() const { return Handle != -1; }

private:
	int Handle;
};

struct mode_info
{
	mode_info()
		: width(oDEFAULT)
		, height(oDEFAULT)
		, depth(oDEFAULT)
		, refresh_rate(oDEFAULT)
	{}

	// resolution
	int width;
	int height;
		
	// usually 16- or 32-bit. oDEFAULT implies current settings
	int depth;

	// usually 60, 75, 85, 120, 240. oDEFAULT implies current settings
	int refresh_rate;
};

struct info
{
	info()
		: native_handle(nullptr)
		, x(oDEFAULT)
		, y(oDEFAULT)
		, workarea_x(oDEFAULT)
		, workarea_y(oDEFAULT)
		, workarea_width(oDEFAULT)
		, workarea_height(oDEFAULT)
		, is_primary(false)
		, is_power_on(false)
	{}
	
	mode_info mode;
	void* native_handle; // HMONITOR on Windows
	int x;
	int y;
	int workarea_x;
	int workarea_y;
	int workarea_width;
	int workarea_height;
	id display_id;
	bool is_primary;
	bool is_power_on;
};

id primary_id();

id get_id(void* _NativeHandle);

int count();

id find(int _ScreenX, int _ScreenY);

info get_info(id _ID);

void enumerate(const std::function<bool(const info& _Info)>& _Enumerator);

void set_mode(id _ID, const mode_info& _Mode);

// restores state after a call to set_mode
void reset_mode(id _ID);

void virtual_rect(int* _pX, int* _pY, int* _pWidth, int* _pHeight);

void taskbar_rect(int* _pX, int* _pY, int* _pWidth, int* _pHeight);

// turns all monitors on or sets them to a low-power state
void set_power_on(bool _On = true);

	} // namespace display
} // namespace ouro

#endif
