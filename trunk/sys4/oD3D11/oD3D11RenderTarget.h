// $(header)
#pragma once
#ifndef oD3D11RenderTarget_h
#define oD3D11RenderTarget_h

#include <SYS4/SYS4Render.h>
#include "SYS4ResourceBaseMixin.h"

struct oD3D11RenderTarget : oGPURenderTarget, oNoncopyable, SYS4ResourceBaseMixin<oGPURenderTarget, oD3D11RenderTarget, oGPUResource::RENDERTARGET>
{
	SYS4_DEFINE_GPURESOURCE_INTERFACE();
	
	oD3D11RenderTarget(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, bool* _pSuccess);

	oRef<oD3D11RenderTarget> RenderTarget;
};

#endif
