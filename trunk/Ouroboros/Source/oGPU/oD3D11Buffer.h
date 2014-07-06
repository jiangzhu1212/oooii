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
#ifndef oGPU_buffer_h
#define oGPU_buffer_h

#include "oGPUCommon.h"

oGPU_NAMESPACE_BEGIN

oRESOURCE_CLASS(buffer)
{
	oRESOURCE_DECLARATION(buffer)
	inline bool has_counter() const { return gpu::has_counter(Info.type); }
	
	ID3D11UnorderedAccessView* choose_uav(bool _Append = false) const { return Info.type == buffer_type::unordered_structured_append ? const_cast<ID3D11UnorderedAccessView*>(UAVAppend.c_ptr()) : const_cast<ID3D11UnorderedAccessView*>(UAV.c_ptr()); }

	void* get_buffer() const override { return (void*)Buffer.c_ptr(); }

	intrusive_ptr<ID3D11Buffer> Buffer;
	intrusive_ptr<ID3D11UnorderedAccessView> UAV;
	intrusive_ptr<ID3D11UnorderedAccessView> UAVAppend;
	intrusive_ptr<ID3D11ShaderResourceView> SRV;
};

oGPU_NAMESPACE_END

#endif
