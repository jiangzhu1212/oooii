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
// Set/Get a name that shows up in driver tty.
#pragma once
#ifndef oGPU_d3d_debug_h
#define oGPU_d3d_debug_h

#include "d3d_types.h"

namespace ouro {
	namespace gpu {
		namespace d3d {

// Set/Get a name that appears in D3D11's debug layer
void debug_name(Device* _pDevice, const char* _Name);
void debug_name(DeviceChild* _pDeviceChild, const char* _Name);

char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const Device* _pDevice);
template<size_t size> char* debug_name(char (&_StrDestination)[size], const Device* _pDevice) { return debug_name(_StrDestination, size, _pDevice); }
template<size_t capacity> char* debug_name(fixed_string<char, capacity>& _StrDestination, const Device* _pDevice) { return debug_name(_StrDestination, _StrDestination.capacity(), _pDevice); }

// Fills the specified buffer with the string set with oD3D11SetDebugName().
char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const	DeviceChild* _pDeviceChild);
template<size_t size> char* debug_name(char (&_StrDestination)[size], const DeviceChild* _pDeviceChild) { return debug_name(_StrDestination, size, _pDeviceChild); }
template<size_t capacity> char* debug_name(fixed_string<char, capacity>& _StrDestination, const DeviceChild* _pDeviceChild) { return debug_name(_StrDestination, _StrDestination.capacity(), _pDeviceChild); }

		} // namespace d3d
	} // namespace gpu
} // namespace ouro

#endif
