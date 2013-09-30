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
#pragma once
#ifndef oBase_image_h
#define oBase_image_h

#include <oBase/color.h>
#include <oBase/surface_codec.h>

namespace ouro {

class image
{
public:
	enum format // only a subset of surface formats are supported as images
	{
		unknown = surface::unknown,
		rgba32 = surface::r8g8b8a8_unorm,
		rgb24 = surface::r8g8b8_unorm,
		bgra32 = surface::b8g8r8a8_unorm,
		bgr24 = surface::b8g8r8_unorm,
		r8 = surface::r8_unorm,
	};

	struct info
	{
		info()
			: format(bgra32)
			, row_pitch(0)
			, dimensions(0, 0)
		{}

		enum format format;
		uint row_pitch;
		int2 dimensions;
	};

	static std::shared_ptr<image> make(const info& _Info);

	virtual info get_info() const = 0;

	virtual void copy_from(const void* _pSourceBuffer, size_t _SourceRowPitch, bool _FlipVertically = false) = 0;
	virtual void copy_to(void* _pDestinationBuffer, size_t _DestinationRowPitch, bool _FlipVertically = false) const = 0;

	virtual void convert(format _NewFormat) = 0;

	virtual std::shared_ptr<surface::buffer> buffer() = 0;
	virtual std::shared_ptr<const surface::buffer> buffer() const = 0;

	// These are non-performant and should only be used for debugging situations
	virtual void put(const int2& _Coordinate, color _Color) = 0;
	virtual color get(const int2& _Coordinate) const = 0;
};

// Returns the pixel data based on the specified buffer that is an in-memory 
// file of one of the supported formats. (For encode pass the underlying surface 
// to surface::encode directly.

std::shared_ptr<image> decode(const void* _pBuffer, size_t _BufferSize, surface::alpha_option::value _Option);

} // namespace ouro

#endif
