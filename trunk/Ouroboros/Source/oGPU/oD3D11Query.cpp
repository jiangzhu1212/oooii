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
#include <oStd/backoff.h>

oDEFINE_GPUDEVICE_CREATE(oD3D11, Query);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, Query)
	, Desc(_Desc)
{
	*_pSuccess = false;
	oD3D11DEVICE();

	switch (Desc.Type)
	{
		case oGPU_QUERY_TIMER:
		{
			static const D3D11_QUERY_DESC sQueryDescs[3] =
			{
				{ D3D11_QUERY_TIMESTAMP, 0 },
				{ D3D11_QUERY_TIMESTAMP, 0 },
				{ D3D11_QUERY_TIMESTAMP_DISJOINT, 0 },
			};
			static_assert(oCOUNTOF(sQueryDescs) <= oCOUNTOF(Queries), "subqueries array cannot hold timer subqueries");

			for (int i = 0; i < oCOUNTOF(sQueryDescs); i++)
			{
				if (FAILED(D3DDevice->CreateQuery(&sQueryDescs[i], &Queries[i])))
				{
					oWinSetLastError();
					return;
				}
			}

			break;
		}

		oNODEFAULT;
	}

	*_pSuccess = true;
}


void oD3D11Query::Begin(ID3D11DeviceContext* _pDeviceContext)
{
	switch (Desc.Type)
	{
		case oGPU_QUERY_TIMER:
			_pDeviceContext->Begin(Queries[TIMER_DISJOINT]);
			_pDeviceContext->End(Queries[TIMER_START]);
			break;
		oNODEFAULT;
	}
}

void oD3D11Query::End(ID3D11DeviceContext* _pDeviceContext)
{
	switch (Desc.Type)
	{
		case oGPU_QUERY_TIMER:
			_pDeviceContext->End(Queries[TIMER_STOP]);
			_pDeviceContext->End(Queries[TIMER_DISJOINT]);
			break;
		oNODEFAULT;
	}
}

static bool PollAsync(ID3D11DeviceContext* _pDeviceContext, ID3D11Asynchronous* _pAsync, void* _pData, uint _SizeofData)
{
	oStd::backoff bo;
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

template<typename T> bool PollAsync(ID3D11DeviceContext* _pDeviceContext, ID3D11Asynchronous* _pAsync, T* _pData)
{
	return PollAsync(_pDeviceContext, _pAsync, _pData, sizeof(T));
}

bool oD3D11Query::ReadQuery(ID3D11DeviceContext* _pDeviceContext, void* _pData, size_t _SizeofData)
{
	switch (Desc.Type)
	{
		case oGPU_QUERY_TIMER:
		{
			if (sizeof(double) != _SizeofData)
				return oErrorSetLast(std::errc::invalid_argument, "Timer query returns a double representing seconds");

			ullong Results[TIMER_COUNT];
			for (int i = 0; i < TIMER_DISJOINT; i++)
			{
				if (!PollAsync(_pDeviceContext, Queries[i], &Results[i]))
					return false;
			}

			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT Disjoint;

			if (!PollAsync(_pDeviceContext, Queries[TIMER_DISJOINT], &Disjoint, sizeof(Disjoint)))
				return false;

			if (Disjoint.Disjoint)
				return oErrorSetLast(std::errc::protocol_error, "Timer is disjoint (laptop AC unplugged, overheating, GPU throttling)");

			ullong TickDelta = Results[TIMER_STOP] - Results[TIMER_START];
			*(double*)_pData = TickDelta / double(Disjoint.Frequency);

			break;
		}

		oNODEFAULT;
	}

	return true;
}
