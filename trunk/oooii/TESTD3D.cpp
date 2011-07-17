/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oooii/oTest.h>
#include <oooii/oD3D10.h>

struct TESTD3D : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oRef<oD3D10DeviceManager> D3D10Manager;
		oD3D10DeviceManager::DESC Desc;

		oTESTB( oD3D10DeviceManager::Create(Desc, &D3D10Manager), "Failed to create D3D10 manager" );

		oRef<ID3D10Device1> D3DDevice;
		oTESTB( D3D10Manager->EnumDevices(0, &D3DDevice ), "Failed to find a single D3D10 device" );

		// Now try to get the same device by querying for a device that has known rectange
		{
			oRef<IDXGIDevice> DXGIDevice;
			oTESTB( S_OK == D3DDevice->QueryInterface(&DXGIDevice), "Failed to create DXGIDevice" );

			oRef<IDXGIAdapter> Adapter;
			oTESTB( S_OK == DXGIDevice->GetParent( __uuidof(IDXGIAdapter), (void**)&Adapter ), "Failed to get adapter from device" );

			oRef<IDXGIOutput> Output;
			oTESTB( DXGI_ERROR_NOT_FOUND != Adapter->EnumOutputs(0, &Output), "No output found" );

			DXGI_OUTPUT_DESC OutputDesc;
			oTESTB( S_OK == Output->GetDesc(&OutputDesc), "Failed to get output description" );

			RECT SubRect = OutputDesc.DesktopCoordinates;
			SubRect.left += (SubRect.right - SubRect.left) / 4;
			SubRect.top += (SubRect.bottom - SubRect.top) / 4;

			oRef<ID3D10Device1> D3DDeviceRegion;
			oTESTB( D3D10Manager->GetDevice(SubRect, &D3DDeviceRegion) && (D3DDeviceRegion.c_ptr() == D3DDevice.c_ptr()), "Failed to find the right device");
		}


		return SUCCESS;
	}
};

oTEST_REGISTER(TESTD3D);