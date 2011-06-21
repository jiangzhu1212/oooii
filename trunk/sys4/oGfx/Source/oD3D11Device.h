// $(header)
#pragma once
#ifndef oD3D11Device_h
#define oD3D11Device_h

#include <oGfx/oGfx.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oWindows.h>
#include <oooii/oD3D11.h>
#include <vector>

#define oD3D11DEVICE() \
	oRef<ID3D11Device> D3DDevice; \
	oVERIFY(Device->QueryInterface(oGetGUID<ID3D11Device>(), &D3DDevice));

const oGUID& oGetGUID(threadsafe const ID3D11Device* threadsafe const *);

struct oD3D11Device : oGfxDevice, oNoncopyable
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oGetGUID<oD3D11Device>(), oGetGUID<oGfxDevice>());

	oD3D11Device(ID3D11Device* _pDevice, const oGfxDevice::DESC& _Desc, bool* _pSuccess);

	bool CreateCommandList(const char* _Name, const oGfxCommandList::DESC& _Desc, oGfxCommandList** _ppCommandList) threadsafe override;
	void CreatePipeline(const char* _Name, const oGfxPipeline::DESC& _Desc, oGfxPipeline** _ppPipeline) threadsafe override;
	bool CreateRenderTarget2(const char* _Name, const oGfxRenderTarget2::DESC& _Desc, oGfxRenderTarget2** _ppRenderTarget) threadsafe override;
	bool CreateMaterial(const char* _Name, const oGfxMaterial::DESC& _Desc, oGfxMaterial** _ppMaterial) threadsafe override;
	bool CreateMesh(const char* _Name, const oGfxMesh::DESC& _Desc, oGfxMesh** _ppMesh) threadsafe override;
	bool CreateTexture(const char* _Name, const oGfxTexture::DESC& _Desc, oGfxTexture** _ppTexture) threadsafe override;

	void Submit() override;

	// API called by other oD3D11* objects
	void Insert(oGfxCommandList* _pCommandList) threadsafe;
	void Remove(oGfxCommandList* _pCommandList) threadsafe;
	void DrawCommandLists() threadsafe;

	oRef<ID3D11Device> D3DDevice;
	oRef<ID3D11DeviceContext> ImmediateContext;

	DESC Desc;
	oRefCount RefCount;

	oMutex CommandListsMutex;
	std::vector<oGfxCommandList*> CommandLists; // non-oRefs to avoid circular refs
	inline std::vector<oGfxCommandList*>& ProtectedCommandLists() threadsafe { return thread_cast<std::vector<oGfxCommandList*>&>(CommandLists); } // safe because this should only be used when protected by CommandListsMutex

	oD3D11RasterizerState RSState;
	oD3D11BlendState OMState;
	oD3D11DepthStencilState DSState;
	oD3D11SamplerState SAState;

	//oD3D11ShaderState PipelineShaderState;
	//oD3D11ShaderState DebugShaderState;
};

#endif
