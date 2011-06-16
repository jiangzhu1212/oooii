// $(header)
#pragma once
#ifndef oD3D11Context_h
#define oD3D11Context_h

#include <SYS4/SYS4Render.h>
#include <oooii/oNoncopyable.h>

struct oD3D11Context : oGPUDeviceContext, oNoncopyable, SYS4ResourceBaseMixin<oGPUContext, oD3D11Context, oGPUResource::CONTEXT>
{
	oD3D11Context(threadsafe oGPUDevice* _pDevice, const DESC& _Desc, bool* _pSuccess);
	~oD3D11Context();

	void Begin(const RENDER_STATE& _RenderState) override;
	void End() override;

	oRef<ID3D11DeviceContext> Context;
	oRef<ID3D11CommandList> Commands;
};

#endif
