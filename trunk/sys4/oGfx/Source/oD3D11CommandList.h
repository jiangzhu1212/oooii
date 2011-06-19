// $(header)
#pragma once
#ifndef oD3D11Context_h
#define oD3D11Context_h

#include <oGfx/oGfx.h>
#include "oGfxResourceMixin.h"
#include <oooii/oNoncopyable.h>

struct oD3D11Context : oGfxDeviceContext, oGfxResourceMixin<oGfxContext, oD3D11Context, oGfxResource::CONTEXT>
{
	oDEFINE_GFXRESOURCE_INTERFACE();

	oD3D11Context(threadsafe oGfxDevice* _pDevice, const DESC& _Desc, bool* _pSuccess);
	~oD3D11Context();

	void Begin(const RENDER_STATE& _RenderState) override;
	void End() override;

	oRef<ID3D11DeviceContext> Context;
	oRef<ID3D11CommandList> Commands;
};

#endif
