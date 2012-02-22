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
#pragma once
#ifndef oD3D11Device_h
#define oD3D11Device_h

#include <oGfx/oGfx.h>
#include <oGfx/oGfxDrawConstants.h>
#include <oBasis/oNoncopyable.h>
#include <oPlatform/oWindows.h>
#include <oPlatform/oD3D11.h>
#include <vector>

#define oD3D11DEVICE() \
	oRef<ID3D11Device> D3DDevice; \
	oVERIFY(Device->QueryInterface(oGetGUID<ID3D11Device>(), &D3DDevice));

// Call reg/unreg from device children implementations
#define oDEVICE_REGISTER_THIS() static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLInsert(this)
#define oDEVICE_UNREGISTER_THIS() static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLRemove(this)

#define oDEVICE_LOCK_SUBMIT() static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLLockSubmit()
#define oDEVICE_UNLOCK_SUBMIT() static_cast<threadsafe oD3D11Device*>(Device.c_ptr())->CLUnlockSubmit()

const oGUID& oGetGUID(threadsafe const ID3D11Device* threadsafe const *);

struct oD3D11Device : oGfxDevice, oNoncopyable
{
	// _____________________________________________________________________________
	// Interface API

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	oD3D11Device(ID3D11Device* _pDevice, const oGfxDevice::DESC& _Desc, bool* _pSuccess);

	bool CreateCommandList(const char* _Name, const oGfxCommandList::DESC& _Desc, oGfxCommandList** _ppCommandList) threadsafe override;
	bool CreateLineList(const char* _Name, const oGfxLineList::DESC& _Desc, oGfxLineList** _ppLineList) threadsafe override;
	bool CreatePipeline(const char* _Name, const oGfxPipeline::DESC& _Desc, oGfxPipeline** _ppPipeline) threadsafe override;
	bool CreateRenderTarget(const char* _Name, const oGfxRenderTarget::DESC& _Desc, oGfxRenderTarget** _ppRenderTarget) threadsafe override;
	bool CreateRenderTarget(const char* _Name, threadsafe oWindow* _pWindow, oSURFACE_FORMAT _DepthStencilFormat, oGfxRenderTarget** _ppRenderTarget) threadsafe override;
	//bool CreateMaterial(const char* _Name, const oGfxMaterial::DESC& _Desc, oGfxMaterial** _ppMaterial) threadsafe override;
	bool CreateMesh(const char* _Name, const oGfxMesh::DESC& _Desc, oGfxMesh** _ppMesh) threadsafe override;
	//bool CreateInstanceList(const char* _Name, const oGfxInstanceList::DESC& _Desc, oGfxInstanceList** _ppInstanceList) threadsafe override;
	//bool CreateTexture(const char* _Name, const oGfxTexture::DESC& _Desc, oGfxTexture** _ppTexture) threadsafe override;

	bool BeginFrame() threadsafe override;
	void EndFrame() threadsafe override;

	uint GetFrameID() const threadsafe override;


	// _____________________________________________________________________________
	// Implementation API

	void CLInsert(oGfxCommandList* _pCommandList) threadsafe;
	void CLRemove(oGfxCommandList* _pCommandList) threadsafe;
	void CLLockSubmit() threadsafe;
	void CLUnlockSubmit() threadsafe;
	inline uint IncrementDrawID() threadsafe { return oStd::atomic_increment(&DrawID); }
	inline uint GetDrawID() const threadsafe { return DrawID; }
	void DrawCommandLists() threadsafe;

	inline std::vector<oGfxCommandList*>& ProtectedCommandLists() threadsafe { return thread_cast<std::vector<oGfxCommandList*>&>(CommandLists); } // safe because this should only be used when protected by CommandListsMutex
#if 0
	// _____________________________________________________________________________
	// Members
#endif
	oRef<ID3D11Device> D3DDevice;
	oRef<ID3D11DeviceContext> ImmediateContext;
	// @oooii-tony:
	// This repeats what's in oD3D11, but this is designed to replace the oD3D11 components,
	// so this will be favored eventually...
	oRef<ID3D11BlendState> OMStates[oOMNUMSTATES];
	oRef<ID3D11RasterizerState> RSStates[oRSNUMSTATES];
	oRef<ID3D11DepthStencilState> DSStates[oDSNUMSTATES];
	oRef<ID3D11SamplerState> SAStates[oSANUMSTATES][oMBNUMSTATES];

	oRef<ID3D11Buffer> ViewConstants;
	oRef<ID3D11Buffer> DrawConstants;

	DESC Desc;
	oRefCount RefCount;
	uint DrawID;
	uint FrameID;

	oMutex CommandListsInsertRemoveMutex;
	oSharedMutex CommandListsBeginEndMutex;
	std::vector<oGfxCommandList*> CommandLists; // non-oRefs to avoid circular refs

	//oD3D11ShaderState PipelineShaderState;
	//oD3D11ShaderState DebugShaderState;
};

#endif