/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// Cross-platform API for the major vocabulary of 3D rendering while trying to 
// remain policy-agnostic.
#pragma once
#ifndef oGPU_h
#define oGPU_h

#include <oBasis/oGPUConcepts.h>
#include <oBasis/oBuffer.h>
#include <oSurface/surface.h>
#include <oGUI/window.h>

// Main SW abstraction for a graphics processor
interface oGPUDevice;

// Main SW abstraction for a single thread of graphics command preparation
interface oGPUDeviceContext;

// {E4BEBD80-C7D1-4470-995E-041116E09BBE}
oDEFINE_GUID_I(oGPUDeviceChild, 0xe4bebd80, 0xc7d1, 0x4470, 0x99, 0x5e, 0x4, 0x11, 0x16, 0xe0, 0x9b, 0xbe);
interface oGPUDeviceChild : oInterface
{
	// Anything allocated from oGPUDevice is an oGPUDeviceChild

	// fill the specified pointer with this resources's associated device. The 
	// device's ref count is incremented.
	virtual void GetDevice(oGPUDevice** _ppDevice) const threadsafe = 0;

	// Returns the identifier as specified at create time.
	virtual const char* GetName() const threadsafe = 0;
};

// {D5A0E41C-AB91-496E-8D5D-A335A92778A2}
oDEFINE_GUID_I(oGPUResource, 0xd5a0e41c, 0xab91, 0x496e, 0x8d, 0x5d, 0xa3, 0x35, 0xa9, 0x27, 0x78, 0xa2);
interface oGPUResource : oGPUDeviceChild
{
	// Anything that contains data intended primarily for read-only access by the 
	// GPU processor is a resource. This does not exclude write access, but 
	// generally differentiates these objects from process and target interfaces.

	// Returns the type of this resource.
	virtual oGPU_RESOURCE_TYPE GetType() const threadsafe = 0;

	// Returns an ID for this resource fit for use as a hash.
	virtual unsigned int GetID() const threadsafe = 0;

	// Returns the component sizes of a subresource. X is the RowSize, or the 
	// number of valid bytes in one scanline of a texture, or the size of one 
	// element of any other buffer. Y is the number of scanlines/rows in a 
	// texture, or the number of elements in the buffer.
	virtual int2 GetByteDimensions(int _Subresource) const threadsafe = 0;
};

// {E4B3CB37-2FD7-4BF5-8CBB-6923F0185A51}
oDEFINE_GUID_I(oGPUBuffer, 0xe4b3cb37, 0x2fd7, 0x4bf5, 0x8c, 0xbb, 0x69, 0x23, 0xf0, 0x18, 0x5a, 0x51);
interface oGPUBuffer : oGPUResource
{
	typedef oGPU_BUFFER_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

// {19374525-0CC8-445B-80ED-A6D2FF13362C}
oDEFINE_GUID_I(oGPUTexture, 0x19374525, 0xcc8, 0x445b, 0x80, 0xed, 0xa6, 0xd2, 0xff, 0x13, 0x36, 0x2c);
interface oGPUTexture : oGPUResource
{
	// A large buffer filled with surface data. Most often this is one or more 
	// 2D planes wrapped onto the surface of a screen or mesh.

	typedef oGPU_TEXTURE_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

// {E7F8FD41-737A-4AC5-A3C0-EB04876C6071}
oDEFINE_GUID_I(oGPURenderTarget, 0xe7f8fd41, 0x737a, 0x4ac5, 0xa3, 0xc0, 0xeb, 0x4, 0x87, 0x6c, 0x60, 0x71);
interface oGPURenderTarget : oGPUDeviceChild
{
	// A 2D plane onto which rendering occurs

	typedef oGPU_RENDER_TARGET_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Modifies the values for clearing without modifying other topology
	virtual void SetClearDesc(const oGPU_CLEAR_DESC& _ClearDesc) threadsafe = 0;

	inline void SetClearColor(int _Index, ouro::color _Color) threadsafe
	{
		DESC d;
		GetDesc(&d);
		d.ClearDesc.ClearColor[_Index] = _Color;
		SetClearDesc(d.ClearDesc);
	}

	inline void SetClearColor(ouro::color _Color) threadsafe { SetClearColor(0, _Color); }

	// Resizes all buffers without changing formats or other topology
	virtual void Resize(const int3& _NewDimensions) = 0;

	// Accesses a readable texture for the specified render target in an MRT.
	virtual void GetTexture(int _MRTIndex, oGPUTexture** _ppTexture) = 0;

	// Accesses a readable texture for the depth-stencil buffer. This can fail if 
	// there is no depth-stencil buffer or if the buffer is of a non-readable 
	// format.
	virtual void GetDepthTexture(oGPUTexture** _ppTexture) = 0;

	// Creates a buffer of the contents of the render target. This should be 
	// called at times when it is known the render target has been fully resolved,
	// mostly outside of BeginFrame/EndFrame. If this is called on the primary
	// render target, the back-buffer is captured.
	virtual std::shared_ptr<ouro::surface::buffer> CreateSnapshot(int _MRTIndex) = 0;
};

// {2401B122-EB19-4CEF-B3BE-9543C003B896}
oDEFINE_GUID_I(oGPUPipeline, 0x2401b122, 0xeb19, 0x4cef, 0xb3, 0xbe, 0x95, 0x43, 0xc0, 0x3, 0xb8, 0x96);
interface oGPUPipeline : oGPUDeviceChild
{
	// A pipeline is the result of setting all stages of the programmable pipeline 
	// (all shaders) and the vertex input format to that pipeline.

	typedef oGPU_PIPELINE_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

// {F095BC1F-872D-4F5E-B962-D91206DD154A}
oDEFINE_GUID_I(oGPUComputeShader, 0xf095bc1f, 0x872d, 0x4f5e, 0xb9, 0x62, 0xd9, 0x12, 0x6, 0xdd, 0x15, 0x4a);
interface oGPUComputeShader : oGPUDeviceChild
{
	// That other pipeline moder GPUs support. This is the CUDA/Compute/OpenCL 
	// path that ignores fixed-function rasterization stages and exposes more 
	// general-purpose components.

	typedef oGPU_COMPUTE_SHADER_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

// {5408DEEA-6A3B-4FFD-B55A-65C340D55A99}
oDEFINE_GUID_I(oGPUQuery, 0x5408deea, 0x6a3b, 0x4ffd, 0xb5, 0x5a, 0x65, 0xc3, 0x40, 0xd5, 0x5a, 0x99);
interface oGPUQuery : oGPUDeviceChild
{
	typedef oGPU_QUERY_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

// {272A9B3E-64BC-4D20-845E-D4EE3F0ED890}
oDEFINE_GUID_I(oGPUCommandList, 0x272a9b3e, 0x64bc, 0x4d20, 0x84, 0x5e, 0xd4, 0xee, 0x3f, 0xe, 0xd8, 0x90);
interface oGPUCommandList : oGPUDeviceChild
{
	// A container for a list of commands issued by the user to the graphics 
	// device. All operations herein are single-threaded. For parallelism separate 
	// command lists can be built in different threads.

	typedef oGPU_COMMAND_LIST_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Begins recording of GPU command submissions. All rendering for this context 
	// should occur between Begin() and End().
	virtual void Begin() = 0;

	// Ends recording of GPU submissions and caches a command list
	virtual void End() = 0;

	virtual void BeginQuery(oGPUQuery* _pQuery) = 0;

	virtual void EndQuery(oGPUQuery* _pQuery) = 0;

	// This should never be required to be called, and has bad performance 
	// implications if called, but is sometimes required, especially with the 
	// immediate context, during debugging.
	virtual void Flush() = 0;

	// Clears the command list's idea of state back to its default. In general 
	// client code should set state absolute state, but if a command list is 
	// reassigned to a different responsibility, it may make sense to start with
	// a clean slate.
	virtual void Reset() = 0;

	// Allocates internal device memory that can be written to (not read) and 
	// committed to the device to update the specified resource.
	virtual void Reserve(oGPUResource* _pResource, int _Subresource, ouro::surface::mapped_subresource* _pMappedSubresource) = 0;

	// Commits memory to the specified resource. If the memory in _Source.pData 
	// was reserved, then this will free the memory. If _Source.pData is user 
	// memory, it will not be freed. If the specified rectangle is empty on any 
	// dimension, then the entire surface will be copied (default behavior). If 
	// the item is an oGPUBuffer then units are in structs, i.e. Left=0, 
	// Right=ArraySize would be a full copy. Ensure that the other dimension are
	// not empty/equal even in the buffer case.
	virtual void Commit(oGPUResource* _pResource, int _Subresource, const ouro::surface::mapped_subresource& _Source, const ouro::surface::box& _Subregion = ouro::surface::box()) = 0;
	inline void Commit(oGPUResource* _pResource, int _Subresource, const ouro::surface::const_mapped_subresource& _Source, const ouro::surface::box& _Subregion = ouro::surface::box()) { Commit(_pResource, _Subresource, (const ouro::surface::mapped_subresource&)_Source, _Subregion); }

	// Copies the contents from one resource to another. Both must have compatible 
	// (often identical) topologies. A common use of this API is to copy from a 
	// source resource to a readback copy of the same resource so it can be 
	// accessed from the CPU.
	virtual void Copy(oGPUResource* _pDestination, oGPUResource* _pSource) = 0;

	// Copies from one buffer to another with offsets in bytes.
	virtual void Copy(oGPUBuffer* _pDestination, int _DestinationOffsetBytes, oGPUBuffer* _pSource, int _SourceOffsetBytes, int _SizeBytes) = 0;

	// Copy the counter value stored in a source buffer of type 
	// oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND or 
	// oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER to a destination buffer. An offset
	// into that buffer can be specified - it must be aligned to sizeof(uint).
	// To read back this value to the CPU, _pDestination should be a READBACK 
	// buffer of at least sizeof(uint) size. Then use MapRead() on the device to
	// access the uint counter value.
	virtual void CopyCounter(oGPUBuffer* _pDestination, uint _DestinationAlignedOffset, oGPUBuffer* _pUnorderedSource) = 0;

	// Sets the counter in the specified buffer to the specified value. This 
	// incurs a dispatch and should not be used in the main loop of production 
	// code. This is exposed primarily for test cases and initialization. For main 
	// loop code use the _pInitialCounts parameter of SetUnorderedResources or 
	// SetRenderTargetAndUnorderedResources. REMEMBER: This will occur when the 
	// command list is submitted to the device during EndFrame(). If the desire is
	// immediate, ensure this command list is the one retrieved from 
	// oGPUDevice::GetImmediateCommandList().
	virtual void SetCounters(int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pValues) = 0;
	inline void SetCounters(int _NumUnorderedBuffers, oGPUBuffer** _ppUnorderedBuffers, uint* _pValues) { SetCounters(_NumUnorderedBuffers, (oGPUResource**)_ppUnorderedBuffers, _pValues); }
	inline void SetCounter(oGPUResource* _pResource, uint _Value) { SetCounters(1, &_pResource, &_Value); }

	// Set the texture sampler states in this context
	virtual void SetSamplers(int _StartSlot, int _NumStates, const oGPU_SAMPLER_STATE* _pSamplerState) = 0;

	// Set any shader resources (textures or buffers not accessed as constants)
	virtual void SetShaderResources(int _StartSlot, int _NumResources, const oGPUResource* const* _ppResources) = 0;
	inline void SetShaderResources(int _StartSlot, int _NumResources, const oGPUTexture* const* _ppResources) { SetShaderResources(_StartSlot, _NumResources, (const oGPUResource* const*)_ppResources); }
	inline void SetShaderResources(int _StartSlot, int _NumResources, const oGPUBuffer* const* _ppResources) { SetShaderResources(_StartSlot, _NumResources, (const oGPUResource* const*)_ppResources); }
	
	template<size_t size> void SetShaderResources(int _StartSlot, const oGPUResource* const (&_ppResources)[size]) { SetShaderResources(_StartSlot, size, (const oGPUBuffer* const*)_ppResources); }
	template<size_t size> void SetShaderResources(int _StartSlot, const oGPUBuffer* const (&_ppResources)[size]) { SetShaderResources(_StartSlot, size, (const oGPUBuffer* const*)_ppResources); }
	template<size_t size> void SetShaderResources(int _StartSlot, const oGPUTexture* const (&_ppResources)[size]) { SetShaderResources(_StartSlot, size, (const oGPUBuffer* const*)_ppResources); }

	template<size_t size> void SetShaderResources(int _StartSlot, const ouro::intrusive_ptr<oGPUResource> (&_ppResources)[size]) { SetShaderResources(_StartSlot, size, (const oGPUBuffer* const*)_ppResources); }
	template<size_t size> void SetShaderResources(int _StartSlot, const ouro::intrusive_ptr<oGPUBuffer> (&_ppResources)[size]) { SetShaderResources(_StartSlot, size, (const oGPUBuffer* const*)_ppResources); }
	template<size_t size> void SetShaderResources(int _StartSlot, const ouro::intrusive_ptr<oGPUTexture> (&_ppResources)[size]) { SetShaderResources(_StartSlot, size, (const oGPUBuffer* const*)_ppResources); }
	
	inline void SetShaderResources(int _StartSlot, const oGPUResource* _pResource) { SetShaderResources(_StartSlot, 1, &_pResource); }
	inline void SetShaderResources(int _StartSlot, const oGPUTexture* _pResource) { SetShaderResources(_StartSlot, 1, &_pResource); }

	// Set the constants in this context
	virtual void SetBuffers(int _StartSlot, int _NumBuffers, const oGPUBuffer* const* _ppBuffers) = 0;

	template<size_t size> inline void SetBuffers(int _StartSlot, const oGPUBuffer* const (&_ppBuffers)[size]) { SetBuffers(_StartSlot, size, _ppBuffers); }
	inline void SetBuffers(int _StartSlot, const oGPUBuffer* _pBuffer) { SetBuffers(_StartSlot, 1, (const oGPUBuffer* const *)&_pBuffer); }

	// Sets the render target to which rendering will occur. By default, a single
	// full-target viewport is created, else it can be overridden. A viewport is 
	// a 3D box whose minimum is at the top, left, near corner of the viewable 
	// frustum, and whose maximum is at the bottom, right, far corner of the 
	// viewable frustum. A full-target viewport would most often be: 
	// oAABoxf(float3(0.0f), float3(RTWidth, RTHeight, 1.0f)). In addition this
	// also sets up unordered resources resetting any counters according to values 
	// in the _pInitialCounts array, which should be the same count as the number 
	// of unordered resources being bound. This API can also bound unordered 
	// resources for access during Dispatch() calls if _SetForDispatch is true. 
	// If _SetForDispatch is true, then _pRenderTarget must be null, though 
	// viewport settings will be respected. If _UnorderedResourceStartSlot is 
	// oInvalid, the value _pRenderTarget::DESC.MRTCount will be used. If 
	// _pRenderTarget is nullptr then a valid _UnorderedResourceStartSlot value 
	// must be specified. If _NumUnorderedResources is oInvalid, all slots after 
	// the render target's MRTs are cleared. See SetRnederTarget() as an example 
	// of setting the RT while clearing unordered targets).
	virtual void SetRenderTargetAndUnorderedResources(oGPURenderTarget* _pRenderTarget, int _NumViewports, const oAABoxf* _pViewports, bool _SetForDispatch, int _UnorderedResourcesStartSlot, int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pInitialCounts = nullptr) = 0;
	inline void SetRenderTargetAndUnorderedResources(oGPURenderTarget* _pRenderTarget, int _NumViewports, const oAABoxf* _pViewports, bool _SetForDispatch, int _UnorderedResourcesStartSlot, int _NumUnorderedResources, oGPUBuffer** _ppUnorderedResources, uint* _pInitialCounts = nullptr) { SetRenderTargetAndUnorderedResources(_pRenderTarget, _NumViewports, _pViewports, _SetForDispatch, _UnorderedResourcesStartSlot, _NumUnorderedResources, (oGPUResource**)_ppUnorderedResources, _pInitialCounts); }
	template<uint size> inline void SetRenderTargetAndUnorderedResources(oGPURenderTarget* _pRenderTarget, int _NumViewports, const oAABoxf* _pViewports, bool _SetForDispatch, int _UnorderedResourcesStartSlot, oGPUResource* (&_ppUnorderedResources)[size], uint (&_pInitialCounts)[size]) { SetRenderTargetAndUnorderedResources(_pRenderTarget, _NumViewports, _pViewports, _SetForDispatch, _UnorderedResourcesStartSlot, size, _ppUnorderedResources, _pInitialCounts); }
	inline void SetUnorderedResources(int _UnorderedResourcesStartSlot, int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pInitialCounts = nullptr) { SetRenderTargetAndUnorderedResources(nullptr, 0, nullptr, true, _UnorderedResourcesStartSlot, _NumUnorderedResources, _ppUnorderedResources, _pInitialCounts); }
	template<uint size> inline void SetUnorderedResources(int _UnorderedResourcesStartSlot, oGPUResource* (&_ppUnorderedResources)[size]) { SetRenderTargetAndUnorderedResources(nullptr, 0, nullptr, true, _UnorderedResourcesStartSlot, size, _ppUnorderedResources); }
	template<uint size> inline void SetUnorderedResources(int _UnorderedResourcesStartSlot, oGPUResource* (&_ppUnorderedResources)[size], uint (&_pInitialCounts)[size]) { SetRenderTargetAndUnorderedResources(nullptr, 0, nullptr, true, _UnorderedResourcesStartSlot, size, _ppUnorderedResources, _pInitialCounts); }
	inline void SetUnorderedResources(int _UnorderedResourcesStartSlot, int _NumUnorderedResources, oGPUBuffer** _ppUnorderedResources, uint* _pInitialCounts = nullptr) { SetUnorderedResources(_UnorderedResourcesStartSlot, _NumUnorderedResources, (oGPUResource**)_ppUnorderedResources, _pInitialCounts); }

	// Simpler version that clears UAVs since likely during rasterization UAVs 
	// will be used as shader resources, which conflicts with being a target. If
	// this is not the desired behavior, use the above more explicit/complicated 
	// version.
	inline void SetRenderTarget(oGPURenderTarget* _pRenderTarget, int _NumViewports = 0, const oAABoxf* _pViewports = nullptr) { SetRenderTargetAndUnorderedResources(_pRenderTarget, _NumViewports, _pViewports, false, oInvalid, oInvalid, (oGPUResource**)nullptr); }

	inline void ClearRenderTargetAndUnorderedResources() { SetRenderTargetAndUnorderedResources(nullptr, 0, nullptr, false, 0, oGPU_MAX_NUM_UNORDERED_BUFFERS, (oGPUResource**)nullptr); }

	inline void ClearUnorderedResources() { SetRenderTargetAndUnorderedResources(nullptr, 0, nullptr, true, 0, oGPU_MAX_NUM_UNORDERED_BUFFERS, (oGPUResource**)nullptr); } 

	// _____________________________________________________________________________
	// Rasterization-specific

	virtual void SetPipeline(const oGPUPipeline* _pPipeline) = 0;

	// Set the rasterization state in this context
	virtual void SetSurfaceState(oGPU_SURFACE_STATE _State) = 0;

	// Set the output merger (blend) state in this context
	virtual void SetBlendState(oGPU_BLEND_STATE _State) = 0;

	// Set the depth-stencil state in this context
	virtual void SetDepthStencilState(oGPU_DEPTH_STENCIL_STATE _State) = 0;

	// Uses a render target's CLEAR_DESC to clear all associated buffers
	// according to the type of clear specified here. If _pRenderTarget is nullptr
	// the the currently set render target is cleared.
	virtual void Clear(oGPURenderTarget* _pRenderTarget, oGPU_CLEAR _Clear) = 0;

	// Submits the specified geometry inputs for rendering under the current state
	// of the command list. If _pIndices is nullptr, then non-indexed drawing is
	// done, else the indices are used to indirect into the vertex buffers.
	// _StartPrimitive/_NumPrimitives are in the primitive set by the current 
	// pipeline. If an instanced vertex buffer is set, then _StartInstance/
	// _NumInstances can be used for instanced drawing in either indexed or non-
	// indexed drawing.
	virtual void Draw(
		const oGPUBuffer* _pIndices
		, int _StartSlot
		, int _NumVertexBuffers
		, const oGPUBuffer* const* _ppVertexBuffers
		, uint _StartPrimitive
		, uint _NumPrimitives
		, uint _StartInstance = oInvalid
		, uint _NumInstances = oInvalid
	) = 0;

	// Draws points without any bound geometry but with the count provided by a GPU
	// synthesized buffer.
	virtual void Draw(oGPUBuffer* _pDrawArgs, int _AlignedByteOffsetForArgs) = 0;

	virtual void DrawSVQuad(uint _NumInstances = 1) = 0;
	
	virtual bool GenerateMips(oGPURenderTarget* _pRenderTarget) = 0;

	// _____________________________________________________________________________
	// Compute-specific

	// Clears the specified resource created for unordered access.
	virtual void ClearI(oGPUResource* _pUnorderedResource, const uint4 _Values) = 0;

	// Clears the specified resource created for unordered access.
	virtual void ClearF(oGPUResource* _pUnorderedResource, const float4 _Values) = 0;

	// If all 3 values in _ThreadGroupCount are oInvalid, then the counts are 
	// automatically determined by the GPU. Unordered resources can be 
	// oGPUBuffer or oGPUTexture, but each must have been created readied
	// for unordered access. If not null, _pInitialCounts should be 
	// _NumUnorderedResources in length and is used to set the initial value of a 
	// counter or append/consume count for buffers of type 
	// oGPU_BUFFER_UNORDERED_STRUCTURED_APPEND or 
	// oGPU_BUFFER_UNORDERED_STRUCTURED_COUNTER. Specify oInvalid to skip 
	// initialization of an entry, thus retaining any prior value of the counter.
	virtual void Dispatch(oGPUComputeShader* _pComputeShader, const int3& _ThreadGroupCount) = 0;

	// Same as Dispatch above, but it gets its thread group count from oGPUBuffer.
	virtual void Dispatch(oGPUComputeShader* _pComputeShader, oGPUBuffer* _pThreadGroupCountBuffer, int _AlignedByteOffsetToThreadGroupCount) = 0;
};

// {B3B5D7BC-F0C5-48D4-A976-7D41308F7450}
oDEFINE_GUID_I(oGPUDevice, 0xb3b5d7bc, 0xf0c5, 0x48d4, 0xa9, 0x76, 0x7d, 0x41, 0x30, 0x8f, 0x74, 0x50);
interface oGPUDevice : oInterface
{
	// Main SW abstraction for a graphics processor

	typedef oGPU_DEVICE_INIT INIT;
	
	typedef oGPU_DEVICE_DESC DESC;
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
	
	virtual const char* GetName() const threadsafe = 0;
	virtual uint GetFrameID() threadsafe const = 0;

	// Returns a command list with an oInvalid DrawOrder in its DESC that does not
	// lock or wait for BeginFrame/EndFrame. Be very careful with this because it
	// sets state that deferred command lists may not be aware of and does so
	// immediately without care for ordering. This should only be called from a 
	// single thread. Calls to this CommandList need not be inside a BeginFrame/
	// EndFrame. Begin()/End() for the immediate command list will noop, so either
	// they don't need to be called or an immediate command list should be able to 
	// substitute for a regular (deferred) command list without code change 
	// (though timing and order differences are likely).
	virtual void GetImmediateCommandList(oGPUCommandList** _ppCommandList) = 0;

	virtual bool CreatePrimaryRenderTarget(ouro::window* _pWindow, ouro::surface::format _DepthStencilFormat, bool _EnableOSRendering, oGPURenderTarget** _ppPrimaryRenderTarget) = 0;

	virtual bool CreateCommandList(const char* _Name, const oGPUCommandList::DESC& _Desc, oGPUCommandList** _ppCommandList) = 0;
	virtual bool CreatePipeline(const char* _Name, const oGPUPipeline::DESC& _Desc, oGPUPipeline** _ppPipeline) = 0;
	virtual bool CreateComputeShader(const char* _Name, const oGPUComputeShader::DESC& _Desc, oGPUComputeShader** _ppComputeShader) = 0;
	virtual bool CreateQuery(const char* _Name, const oGPUQuery::DESC&, oGPUQuery** _ppQuery) = 0;
	virtual bool CreateRenderTarget(const char* _Name, const oGPURenderTarget::DESC& _Desc, oGPURenderTarget** _ppRenderTarget) = 0;
	virtual bool CreateBuffer(const char* _Name, const oGPUBuffer::DESC& _Desc, oGPUBuffer** _ppBuffer) = 0;
	virtual bool CreateTexture(const char* _Name, const oGPUTexture::DESC& _Desc, oGPUTexture** _ppTexture) = 0;

	// MapRead is a non-blocking call to read from the specified resource by the
	// mapped data populated in the specified _pMappedSubresource. If the function
	// would block, this returns false. If it succeeds, call ReadEnd to unlock.
	// the buffer. This will also return false for resources not of type READBACK.
	virtual bool MapRead(oGPUResource* _pReadbackResource, int _Subresource, ouro::surface::mapped_subresource* _pMappedSubresource, bool _bBlocking=false) = 0;
	virtual void UnmapRead(oGPUResource* _pReadbackResource, int _Subresource) = 0;

	virtual bool ReadQuery(oGPUQuery* _pQuery, void* _pData, uint _SizeofData) = 0;
	template<typename T> bool ReadQuery(oGPUQuery* _pQuery, T* _pData) { return ReadQuery(_pQuery, _pData, sizeof(T)); }

	virtual bool BeginFrame() = 0;
	virtual void EndFrame() = 0;

	// After out of EndFrame, the device can provide a handle to OS CPU-based 
	// rendering. All OS calls should occur in between BeginOSFrame and EndOSFrame
	// and this should be called after EndFrame, but before Present to ensure no 
	// tearing.
	virtual ouro::draw_context_handle BeginOSFrame() = 0;
	virtual void EndOSFrame() = 0;

	// This should only be called on same thread as the window passed to 
	// CreatePrimaryRenderTarget. If a primary render target does not exist, this
	// will noop, otherwise the entire client area of the associated window will
	// receive the contents of the back buffer. Call this after EndFrame() to 
	// ensure all commands have been flushed.

	virtual bool IsFullscreenExclusive() const = 0;
	virtual bool SetFullscreenExclusive(bool _Fullscreen) = 0;

	virtual bool Present(int _SyncInterval) = 0;
};

oAPI bool oGPUDeviceCreate(const oGPU_DEVICE_INIT& _Init, oGPUDevice** _ppDevice);

// Compiles a shader to its driver-specific bytecode. To make parsing easy, 
// there are two lists of defines that can be specified. These are concatenated
// internally.
oAPI bool oGPUCompileShader(
	const char* _IncludePaths // semi-colon delimited list of paths to look in. 
	                          // Use %DEV% for oSYSPATH_DEV
	, const char* _CommonDefines // semi-colon delimited list of symbols (= value) 
	, const char* _SpecificDefines // semi-colon delimited list of symbols (= value)
	, const ouro::version& _TargetShaderModel // shader model version to compile against
	, oGPU_PIPELINE_STAGE _Stage // type of shader to compile
	, const char* _ShaderPath // full path to shader - mostly for error reporting
	, const char* _EntryPoint // name of the top-level shader function to use
	, const char* _ShaderBody // a string of the loaded shader file (NOTE: This 
	                          // still goes out to includes, so this will still 
	                          // touch the file system.
	, oBuffer** _ppByteCode // if successful, this is a buffer of byte code
	, oBuffer** _ppErrors); // if failure this is filled with an error string

// @tony: I'm not sure if these APIs belong here at all, in oGPU, or 
// oGPUUtil. I do know they don't belong where they were before, which was 
// nowhere.
// Basically rendering at oooii grew up on D3D and D3DX, which have very robust
// libraries for image format conversion and handling. We didn't spend time 
// reproducing this on our own, so we're relying on this stuff as placeholder.
// I suspect if we add support for a different library, such robust support for
// image formats won't be there and we'll have to implement a lot of this stuff,
// so defer that dev time until then and then bring it back to something that
// can be promoted out of oGPU. For now, at least hide the D3D part...

// Convert the format of a surface into another format in another surface. This
// uses GPU acceleration for BC6H/7 and is currently a pass-through to D3DX11's
// conversion functions at the moment. Check debug logs if this function seems
// to hang because if for whatever reason the CPU/SW version of the BC6H/7
// codec is used, it can take a VERY long time.
oAPI bool oGPUSurfaceConvert(
	void* oRESTRICT _pDestination
	, uint _DestinationRowPitch
	, ouro::surface::format _DestinationFormat
	, const void* oRESTRICT _pSource
	, uint _SourceRowPitch
	, ouro::surface::format _SourceFormat
	, const int2& _MipDimensions);

// Extract the parameters for the above call directly from textures
oAPI bool oGPUSurfaceConvert(oGPUTexture* _pSourceTexture, ouro::surface::format _NewFormat, oGPUTexture** _ppDestinationTexture);

// Loads a texture from disk. The _Desc specifies certain conversions/resizes
// that can occur on load. Use oDEFAULT or ouro::surface::unknown to use values as 
// they are found in the specified image resource/buffer.
// @tony: At this time the implementation does NOT use oImage loading 
// code plus a simple call to call oGPUCreateTexture(). Because this API 
// supports conversion for any ouro::surface::format, at this time we defer to 
// DirectX's .dds support for advanced formats like BC6 and BC7 as well as their
// internal conversion library. When it's time to go cross-platform, we'll 
// revisit this and hopefully call more generic code.
oAPI bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _URIReference, const char* _DebugName, oGPUTexture** _ppTexture);
oAPI bool oGPUTextureLoad(oGPUDevice* _pDevice, const oGPU_TEXTURE_DESC& _Desc, const char* _DebugName, const void* _pBuffer, size_t _SizeofBuffer, oGPUTexture** _ppTexture);

enum oGPU_FILE_FORMAT
{
	oGPU_FILE_FORMAT_DDS,
	oGPU_FILE_FORMAT_JPG,
	oGPU_FILE_FORMAT_PNG,
};

// Saves a texture to disk. The format will be determined from the contents of 
// the texture. If the specified format does not support the contents of the 
// texture the function will return false - check oErrorGetLast() for extended
// information.
oAPI bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, void* _pBuffer, size_t _SizeofBuffer);
oAPI bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, const char* _Path);

#endif
