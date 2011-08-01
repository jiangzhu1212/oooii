// $(header)
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include <oooii/oSurface.h>

bool oD3D11Device::CreateRenderTarget2(const char* _Name, threadsafe oWindow* _pWindow, oGfxRenderTarget2** _ppRenderTarget) threadsafe
{
	oGFXCREATE_CHECK_NAME();
	if (!_pWindow)
	{
		oSetLastError(EINVAL, "A window to associate with this new render target must be specified");
		return false;
	}

	bool success = false;
	oCONSTRUCT(_ppRenderTarget, oD3D11RenderTarget2(this, _pWindow, _Name, &success)); \
	return success;
}

oDEFINE_GFXDEVICE_CREATE(oD3D11, RenderTarget2);
oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(oD3D11, RenderTarget2)
{
	Desc = _Desc;
	// nullify width/height to force allocation in this call to resize
	Desc.Width = 0;
	Desc.Height = 0;
	Resize(_Desc.Width, _Desc.Height);
	*_pSuccess = true;
}

oD3D11RenderTarget2::oD3D11RenderTarget2(threadsafe oGfxDevice* _pDevice, threadsafe oWindow* _pWindow, const char* _Name, bool* _pSuccess)
	: oGfxDeviceChildMixin(_pDevice, _Name)
{
	// Not yet implemented
	*_pSuccess = false;
}

void oD3D11RenderTarget2::GetDesc(DESC* _pDesc) const threadsafe
{
	oRWMutex::ScopedLockRead lock(DescMutex);
	*_pDesc = thread_cast<DESC&>(Desc); // safe because of lock above
}

void oD3D11RenderTarget2::SetClearDesc(const CLEAR_DESC& _ClearDesc) threadsafe
{
	oRWMutex::ScopedLock lock(DescMutex);
	thread_cast<DESC&>(Desc).ClearDesc = _ClearDesc; // safe because of lock above
}

void oD3D11RenderTarget2::Resize(uint _Width, uint _Height)
{
	if (_Width != Desc.Width || _Height != Desc.Height)
	{
		oTRACE("%s %s Resize %ux%u -> %ux%u", typeid(*this), GetName(), Desc.Width, Desc.Height, _Width, _Height);

		if (!Desc.Width || !Desc.Height)
		{
			for (unsigned int i = 0; i < oCOUNTOF(Texture); i++)
			{
				Texture[i] = 0;
				RTVs[i] = 0;
				SRVs[i] = 0;
			}

			Depth = 0;
			DSV = 0;
			SRVDepth = 0;
		}

		else
		{
			//oD3D11DEVICE();
			oRef<ID3D11Device> D3DDevice;

			char name[1024];

			for (unsigned int i = 0; i < Desc.ArraySize; i++)
			{
				sprintf_s(name, "%s%02u", GetName(), i);
				oVERIFY(oD3D11CreateRenderTarget(D3DDevice, name, Desc.Width, Desc.Height, Desc.ArraySize, oSurface::GetPlatformFormat<DXGI_FORMAT>(Desc.Format[i]), &Texture[i], &RTVs[i], &SRVs[i]));
			}

			if (Desc.DepthStencilFormat != DXGI_FORMAT_UNKNOWN)
			{
				sprintf_s(name, "%sDS", GetName());
				oVERIFY(oD3D11CreateRenderTarget(D3DDevice, name, Desc.Width, Desc.Height, Desc.ArraySize, oSurface::GetPlatformFormat<DXGI_FORMAT>(Desc.DepthStencilFormat), &Depth, &DSV, &SRVDepth));
			}
		}

		oRWMutex::ScopedLock lock(DescMutex);
		Desc.Width = _Width;
		Desc.Height = _Height;
	}
}
