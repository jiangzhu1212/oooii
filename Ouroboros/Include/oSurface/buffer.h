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
#ifndef oSurface_buffer_h
#define oSurface_buffer_h

#include <memory>
#include <oSurface/resize.h>

namespace ouro {
	namespace surface {
	
class buffer
{
public:
	static std::shared_ptr<buffer> make(const info& _Info);
	static std::shared_ptr<buffer> make(const info& _Info, void* _pData);

	virtual info get_info() const = 0;
	inline size_t size() const { return total_size(get_info()); }

	// Without modifying the data, this updates the Info to be an image layout 
	// with an array_size of 0. This basically means that saving the buffer to a 
	// file will save the layout of the entire surface.
	virtual void flatten() = 0;

	virtual void update_subresource(int _Subresource, const const_mapped_subresource& _Source, bool _FlipVertically = false) = 0;
	virtual void update_subresource(int _Subresource, const box& _Box, const const_mapped_subresource& _Source, bool _FlipVertically = false) = 0;

	virtual void map(int _Subresource, mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) = 0;
	virtual void unmap(int _Subresource) = 0;

	virtual void map_const(int _Subresource, const_mapped_subresource* _pMapped, int2* _pByteDimensions = nullptr) const = 0;
	virtual void unmap_const(int _Subresource) const = 0;

	virtual void copy_to(int _Subresource, mapped_subresource* _pMapped, bool _FlipVertically = false) const = 0;

	virtual std::shared_ptr<buffer> convert(const info& _ConvertedInfo) const = 0;
	inline std::shared_ptr<buffer> convert(format _NewFormat) const { info si = get_info(); si.format = _NewFormat; return convert(si); }

	// Only for compatible types, such as RGB <-> BGR. This is basically an 
	// in-place conversion.
	virtual void swizzle(format _NewFormat) = 0;

	// Takes the top-level mip and replaces all other mips with new contents.
	virtual void generate_mips(filter::value _Filter = filter::lanczos2) = 0;
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

// Returns the root mean square of the difference between the two surfaces. If
// the formats or sizes are different, this throws an exception.
float calc_rms(const buffer* _pBuffer1, const buffer* _pBuffer2, buffer* _pDifferences = nullptr, int _DifferenceScale = 1);
inline float calc_rms(std::shared_ptr<const buffer>& _pBuffer1, std::shared_ptr<const buffer>& _pBuffer2) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), nullptr, 1); }
inline float calc_rms(std::shared_ptr<buffer>& _pBuffer1, std::shared_ptr<const buffer>& _pBuffer2) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), nullptr, 1); }
inline float calc_rms(std::shared_ptr<const buffer>& _pBuffer1, std::shared_ptr<buffer>& _pBuffer2) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), nullptr, 1); }
inline float calc_rms(std::shared_ptr<buffer>& _pBuffer1, std::shared_ptr<buffer>& _pBuffer2) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), nullptr, 1); }
inline float calc_rms(std::shared_ptr<const buffer>& _pBuffer1, std::shared_ptr<const buffer>& _pBuffer2, std::shared_ptr<buffer>& _pDifferences, int _DifferenceScale = 1) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), _pDifferences.get(), _DifferenceScale); }
inline float calc_rms(std::shared_ptr<buffer>& _pBuffer1, std::shared_ptr<const buffer>& _pBuffer2, std::shared_ptr<buffer>& _pDifferences, int _DifferenceScale = 1) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), _pDifferences.get(), _DifferenceScale); }
inline float calc_rms(std::shared_ptr<const buffer>& _pBuffer1, std::shared_ptr<buffer>& _pBuffer2, std::shared_ptr<buffer>& _pDifferences, int _DifferenceScale = 1) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), _pDifferences.get(), _DifferenceScale); }
inline float calc_rms(std::shared_ptr<buffer>& _pBuffer1, std::shared_ptr<buffer>& _pBuffer2, std::shared_ptr<buffer>& _pDifferences, int _DifferenceScale = 1) { return calc_rms(_pBuffer1.get(), _pBuffer2.get(), _pDifferences.get(), _DifferenceScale); }

	} // namespace surface
} // namespace ouro

#endif
