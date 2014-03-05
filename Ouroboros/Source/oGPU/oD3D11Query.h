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
#ifndef oGPU_query_h
#define oGPU_query_h

#include "oGPUCommon.h"

oGPU_NAMESPACE_BEGIN

oDEVICE_CHILD_CLASS(query)
{
	oDEVICE_CHILD_DECLARATION(query)
	query_info get_info() const override { return Info; }
	void begin(ID3D11DeviceContext* _pDeviceContext);
	void end(ID3D11DeviceContext* _pDeviceContext);
	bool read_query(ID3D11DeviceContext* _pDeviceContext, void* _pData, size_t _SizeofData);

	enum TIMER_QUERIES
	{
		TIMER_START,
		TIMER_STOP,
		TIMER_DISJOINT,
		TIMER_COUNT,
	};

	intrusive_ptr<ID3D11Query> Queries[3];
	query_info Info;
};

oGPU_NAMESPACE_END

#endif
