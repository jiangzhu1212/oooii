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
// Anything that might want to be consistently controlled by a unit test 
// infrastructure is separated out into this virtual interface.
#pragma once
#ifndef oSurfaceTestRequirements_h
#define oSurfaceTestRequirements_h

#include <oBase/path.h>
#include <oSurface/buffer.h>

namespace ouro {
	namespace tests {

interface requirements
{
	// Implements a C-standard rand call
	virtual int rand() = 0;

	virtual void vreport(const char* _Format, va_list _Args) = 0;
	inline void report(const char* _Format, ...) { va_list a; va_start(a, _Format); vreport(_Format, a); va_end(a); }

	// fread's the file's contents and returns the pointer
	virtual std::shared_ptr<char> load_buffer(const path& _Path, size_t* _pSize = nullptr) = 0;

	// This function compares the specified surface to a golden image named after
	// the test's name suffixed with _NthTest. If _NthTest is 0 then the golden 
	// image should not have a suffix. If _MaxRMSError is negative a default 
	// should be used. If the surfaces are not similar this throws an exception.
	virtual void check(const surface::info& _SourceInfo
		, const surface::const_mapped_subresource& _Source
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f) = 0;

	inline void check(const surface::buffer* _pBuffer
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f)
	{
		surface::shared_lock lock(_pBuffer);
		check(_pBuffer->get_info(), lock.mapped, _NthTest, _MaxRMSError);
	}

	inline void check(std::shared_ptr<surface::buffer>& _pBuffer
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f)
	{
		surface::shared_lock lock(_pBuffer.get());
		check(_pBuffer->get_info(), lock.mapped, _NthTest, _MaxRMSError);
	}

	inline void check(std::shared_ptr<const surface::buffer>& _pBuffer
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f)
	{
		surface::shared_lock lock(_pBuffer.get());
		check(_pBuffer->get_info(), lock.mapped, _NthTest, _MaxRMSError);
	}
};

	} // namespace tests
} // namespace ouro

#endif
