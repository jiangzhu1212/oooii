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
#include "oD3D11Device.h"
#include "oD3D11Query.h"
#include <oBase/backoff.h>

oGPU_NAMESPACE_BEGIN

oDEFINE_DEVICE_MAKE(query)
oDEVICE_CHILD_CTOR(query)
	, Info(_Info)
{
	oD3D11_DEVICE();

	switch (Info.type)
	{
		case query_type::timer:
		{
			static const D3D11_QUERY_DESC sQueryDescs[3] =
			{
				{ D3D11_QUERY_TIMESTAMP, 0 },
				{ D3D11_QUERY_TIMESTAMP, 0 },
				{ D3D11_QUERY_TIMESTAMP_DISJOINT, 0 },
			};
			static_assert(oCOUNTOF(sQueryDescs) <= oCOUNTOF(Queries), "subqueries array cannot hold timer subqueries");
			
			for (int i = 0; i < oCOUNTOF(sQueryDescs); i++)
				oV(D3DDevice->CreateQuery(&sQueryDescs[i], &Queries[i]));
			
			break;
		}

		oNODEFAULT;
	}
}

void d3d11_query::begin(ID3D11DeviceContext* _pDeviceContext)
{
	switch (Info.type)
	{
		case gpu::query_type::timer:
			_pDeviceContext->Begin(Queries[TIMER_DISJOINT]);
			_pDeviceContext->End(Queries[TIMER_START]);
			break;
		oNODEFAULT;
	}
}

void d3d11_query::end(ID3D11DeviceContext* _pDeviceContext)
{
	switch (Info.type)
	{
		case gpu::query_type::timer:
			_pDeviceContext->End(Queries[TIMER_STOP]);
			_pDeviceContext->End(Queries[TIMER_DISJOINT]);
			break;
		oNODEFAULT;
	}
}

static bool poll_async(ID3D11DeviceContext* _pDeviceContext, ID3D11Asynchronous* _pAsync, void* _pData, uint _SizeofData)
{
	backoff bo;
	HRESULT hr = S_FALSE;

	while(true)
	{
		hr = _pDeviceContext->GetData(_pAsync, _pData, _SizeofData, 0);
		if(S_FALSE != hr)
			break;
		bo.pause();
	}

	return hr == S_OK;
}

template<typename T> bool poll_async(ID3D11DeviceContext* _pDeviceContext, ID3D11Asynchronous* _pAsync, T* _pData)
{
	return poll_async(_pDeviceContext, _pAsync, _pData, sizeof(T));
}

bool d3d11_query::read_query(ID3D11DeviceContext* _pDeviceContext, void* _pData, size_t _SizeofData)
{
	switch (Info.type)
	{
		case gpu::query_type::timer:
		{
			if (sizeof(double) != _SizeofData)
				oTHROW_INVARG("Timer query returns a double representing seconds");

			ullong Results[TIMER_COUNT];
			for (int i = 0; i < TIMER_DISJOINT; i++)
				if (!poll_async(_pDeviceContext, Queries[i], &Results[i]))
					return false;

			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT Disjoint;

			if (!poll_async(_pDeviceContext, Queries[TIMER_DISJOINT], &Disjoint, sizeof(Disjoint)))
				return false;

			if (Disjoint.Disjoint)
				oTHROW(protocol_error, "Timer is disjoint (laptop AC unplugged, overheating, GPU throttling)");

			ullong TickDelta = Results[TIMER_STOP] - Results[TIMER_START];
			*(double*)_pData = TickDelta / double(Disjoint.Frequency);

			break;
		}

		oNODEFAULT;
	}

	return true;
}

oGPU_NAMESPACE_END