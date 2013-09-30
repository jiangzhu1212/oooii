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
// Uses surface utility functions to manage a buffer filled with all that a 
// surface can support. This is basically a CPU-side version of similar GPU 
// buffers in D3D and OGL.
#pragma once
#ifndef oBase_surface_buffer_h
#define oBase_surface_buffer_h

#include <oBase/surface.h>

namespace ouro {
	namespace surface {
	
class buffer
{
public:
	static std::shared_ptr<buffer> make(const info& _Info);
	static std::shared_ptr<buffer> buffer::make(const info& _Info, void* _pData);

	virtual info get_info() const = 0;

	virtual void update_subresource(int _Subresource, const const_mapped_subresource& _Source, bool _FlipVertically = false) = 0;
	virtual void update_subresource(int _Subresource, const box& _Box, const const_mapped_subresource& _Source, bool _FlipVertically = false) = 0;

	virtual void map(int _Subresource, mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) = 0;
	virtual void unmap(int _Subresource) = 0;

	virtual void map_const(int _Subresource, const_mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) const = 0;
	virtual void unmap_const(int _Subresource) const = 0;

	virtual void copy_to(int _Subresource, mapped_subresource* _pMapped, bool _FlipVertically = false) const = 0;

	virtual std::shared_ptr<buffer> convert(const info& _ConvertedInfo) const = 0;
	inline std::shared_ptr<buffer> convert(format _NewFormat) const { info si = get_info(); si.format = _NewFormat; return std::move(convert(si)); }

	// Only for compatible types, such as RGB <-> BGR. This is basically an 
	// in-place conversion.
	virtual void swizzle(format _NewFormat) = 0;
};

class lock_guard
{
public:
	lock_guard(std::shared_ptr<buffer>& _Buffer, int _Subresource = 0)
		: pBuffer(_Buffer.get())
		, Subresource(_Subresource)
	{ pBuffer->map(Subresource, &mapped, &byte_dimensions); }

	lock_guard(buffer* _pBuffer, int _Subresource = 0)
		: pBuffer(_pBuffer)
		, Subresource(_Subresource)
	{ pBuffer->map(Subresource, &mapped, &byte_dimensions); }

	~lock_guard() { pBuffer->unmap(Subresource); }

	mapped_subresource mapped;
	int2 byte_dimensions;

private:
	buffer* pBuffer;
	int Subresource;
};

class shared_lock
{
public:
	shared_lock(std::shared_ptr<buffer>& _Buffer, int _Subresource = 0)
		: pBuffer(_Buffer.get())
		, Subresource(_Subresource)
	{ pBuffer->map_const(Subresource, &mapped, &byte_dimensions); }

	shared_lock(const buffer* _pBuffer, int _Subresource = 0)
		: pBuffer(_pBuffer)
		, Subresource(_Subresource)
	{ pBuffer->map_const(Subresource, &mapped, &byte_dimensions); }

	~shared_lock() { pBuffer->unmap_const(Subresource); }

	const_mapped_subresource mapped;
	int2 byte_dimensions;

private:
	int Subresource;
	const buffer* pBuffer;
};

	} // namespace surface
} // namespace ouro

#endif
