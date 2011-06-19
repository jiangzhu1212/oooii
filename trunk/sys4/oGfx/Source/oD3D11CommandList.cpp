// $(header)
#include "oD3D11Context.h"
#include <oooii/oD3D11.h>
#include <oooii/oErrno.h>

oDEFINE_GFXRESOURCE_CREATE(Context)
oBEGIN_DEFINE_GFXAPI_CTOR(D3D11, Context)
{
	*_pSuccess = false;
	ID3D11Device* D3DDevice = 0;
	oVERIFY(Device->QueryInterface(oGetGUID(ID3D11Device), (void**)&D3DDevice));

	if (FAILED(D3DDevice->CreateDeferredContext(0, &Context)))
	{
		char err[128];
		sprintf_s(err, "Failed to create oGfxDeviceContext %u: ", _Desc.DrawOrder);
		oSetLastErrorNative(hr, err);
		return;
	}

	static_cast<oD3D11Device*>(Device.c_ptr())->Insert(this);

	*_pSuccess = true;
}

oD3D11Context::~oD3D11Context()
{
	static_cast<oD3D11Device*>(Device.c_ptr())->Remove(this);
}

void oD3D11Context::Begin(const RENDER_STATE& _RenderState)
{
	//struct RENDER_STATE
	//{
	//	float4x4 View;
	//	float4x4 Projection;
	//	PIPELINE_STATE State;
	//	DEBUG_VISUALIZATION DebugVisualization;
	//	oColor ClearColor;
	//	float ClearDepthValue;
	//	unsigned char ClearStencilValue;
	//};
}

void oD3D11Context::End()
{
	Context->FinishCommandList(FALSE, &Commands);
}
