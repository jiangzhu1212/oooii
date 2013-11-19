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
#ifndef oD3D11Query_h
#define oD3D11Query_h

#include <oGPU/oGPU.h>
#include "oGPUCommon.h"
#include "d3d11.h"

// {47A1C5E7-D484-414C-9B23-1587E875F97F}
oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, Query, 0x47a1c5e7, 0xd484, 0x414c, 0x9b, 0x23, 0x15, 0x87, 0xe8, 0x75, 0xf9, 0x7f)
{
	oDEFINE_GPUDEVICECHILD_INTERFACE();
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, Query);
	void Begin(ID3D11DeviceContext* _pDeviceContext);
	void End(ID3D11DeviceContext* _pDeviceContext);
	bool ReadQuery(ID3D11DeviceContext* _pDeviceContext, void* _pData, size_t _SizeofData);
	DESC Desc;

	enum TIMER_QUERIES
	{
		TIMER_START,
		TIMER_STOP,
		TIMER_DISJOINT,
		TIMER_COUNT,
	};

	ouro::intrusive_ptr<ID3D11Query> Queries[3];
};

#endif
