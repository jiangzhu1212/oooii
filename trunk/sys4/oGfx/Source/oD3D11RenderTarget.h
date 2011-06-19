// $(header)
#pragma once
#ifndef oD3D11RenderTarget_h
#define oD3D11RenderTarget_h

#include <oGfx/oGfx.h>
#include "oGfxResourceMixin.h"

struct oD3D11RenderTarget : oGfxRenderTarget, oGfxResourceMixin<oGfxRenderTarget, oD3D11RenderTarget, oGfxResource::RENDERTARGET>
{
	oDEFINE_GFXRESOURCE_INTERFACE();
	oDECLARE_GFXRESOURCE_CTOR(D3D11, RenderTarget);
	oRef<oD3D11RenderTarget> RenderTarget;
};

#endif
