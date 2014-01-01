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
// Abstraction for direct-show cameras.
#pragma once
#ifndef oCore_camera_h
#define oCore_camera_h

#include <functional>
#include <memory>
#include <oSurface/surface.h>

namespace ouro {

class camera
{
public:
	enum format
	{
		unknown = surface::unknown,
		rgb565 = surface::b5g5r5a1_unorm,
		rgb24 = surface::b8g8r8_unorm,
		argb32 = surface::b8g8r8a8_unorm,
		rgb32 = surface::b8g8r8x8_unorm,
	};

	struct mode
	{
		mode()
			: dimensions(0, 0)
			, format(unknown)
			, bit_rate(0)
		{}

		int2 dimensions;
		enum format format;
		int bit_rate;
	};

	struct const_mapped
	{
		const void* data;
		unsigned int row_pitch;
		unsigned int frame;
	};

	virtual const char* name() const = 0;
	virtual unsigned int id() const = 0;
	virtual float fps() const = 0;
	virtual mode get_mode() const = 0;
	virtual void set_mode(const mode& _Mode) = 0;
	virtual bool capturing() const = 0;
	virtual void capturing(bool _Capturing) = 0;

	// Return true from _Enumerator to continue, or false to abort enumeration.
	virtual void enumerate_modes(const std::function<bool(const mode& _Mode)>& _Enumerator) const = 0;

	// returns mode() if none found
	virtual mode find_closest_matching(const mode& _Mode) const = 0;

	virtual bool map_const(const_mapped* _pMapped) const = 0;
	virtual void unmap_const() const = 0;

	// Cameras aren't created so much as retrieved using this enumerator.
	static void enumerate(const std::function<bool(std::shared_ptr<camera> _Camera)>& _Enumerator);
};

} // namespace ouro

#endif