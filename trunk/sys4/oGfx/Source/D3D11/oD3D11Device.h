// $(header)
#pragma once
#ifndef oD3D11Device_h
#define oD3D11Device_h

#include <SYS4/SYS4Render.h>
#include <oooii/oNoncopyable.h>

struct oD3D11Device : oGPUDevice, oNoncopyable
{
	oD3D11Device(ID3D11Device* _pD3DDevice, const oGPUDevice::DESC& _Desc, bool* _pSuccess);

	bool CreateMesh(const char* _Name, const oGPUMesh::DESC& _Desc, threadsafe oGPUMesh** _ppMesh) threadsafe override;
	bool CreateTexture(const char* _Name, const oGPUTexture::DESC& _Desc, threadsafe oGPUTexture** _ppTexture) threadsafe override;
	bool CreateMaterial(const char* _Name, const oGPUMaterial::DESC& _Desc, threadsafe oGPUMaterial** _ppMaterial) threadsafe override;
	bool CreateContext(const char* _Name, oGPUDeviceContext** _ppContext) threadsafe override;

	void Begin() override;
	void End() override;

	// API called by other oD3D11* objects
	void Insert(oGPUContext* _pContext) threadsafe;
	void Remove(oGPUContext* _pContext) threadsafe;

	oRef<ID3D11Device> D3DDevice;
	oRef<ID3D11DeviceContext> ImmediateContext;

	oMutex ContextsMutex;
	oLockedVector<oGPUDeviceContext*> Contexts; // non-oRefs to avoid circular refs

	oD3D11RasterizerState RasterizerState;
	oD3D11BlendState BlendState;
	oD3D11DepthStencilState DepthStencilState;
	oD3D11SamplerState SamplerState;

	oD3D11ShaderState PipelineShaderState;
	//oD3D11ShaderState DebugShaderState;

};

#endif
