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
// Cross-platform API for the major vocabulary of 3D 
// rendering while trying to remain policy-agnostic
#pragma once
#ifndef oGPU_h
#define oGPU_h

#include <oBasis/oGPUEnums.h>
#include <oGPU/oGPUStructs.h>

// Main SW abstraction for a graphics processor
interface oGPUDevice;

// Main SW abstraction for a single thread of graphics command preparation
interface oGPUDeviceContext;

interface oGPUDeviceChild : oInterface
{
	// Anything allocated from oGPUDevice is an oGPUDeviceChild

	// fill the specified pointer with this resources's associated device. The 
	// device's ref count is incremented.
	virtual void GetDevice(oGPUDevice** _ppDevice) const threadsafe = 0;

	// Returns the identifier as specified at create time.
	virtual const char* GetName() const threadsafe = 0;
};

interface oGPUResource : oGPUDeviceChild
{
	// Anything that contains data intended primarily for read-only access by the 
	// GPU processor is a resource. This does not exclude write access, but 
	// generally differentiates these objects from process and target interfaces.

	// Returns the type of this resource.
	virtual oGPU_RESOURCE_TYPE GetType() const threadsafe = 0;

	// Returns an ID for this resource fit for use as a hash.
	virtual int GetID() const threadsafe = 0;

	// Returns the component sizes of a subresource. X is the RowSize, or the 
	// number of valid bytes in one scanline of a texture, or the size of one 
	// element of any other buffer. Y is the number of scanlines/rows in a 
	// texture, or the number of elements in the buffer.
	virtual int2 GetByteDimensions(int _Subresource) const threadsafe = 0;
};

interface oGPUInstanceList : oGPUResource
{
	// Contains instance data for drawing instances of a mesh. The contents of 
	// each instance's data is user-defined using IAELEMENTs. This is more or less 
	// a different semantic of vertex attributes.

	struct DESC : oGPU_INSTANCE_LIST_DESC {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPULineList : oGPUResource
{
	struct DESC : oGPU_LINE_LIST_DESC {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUBuffer : oGPUResource
{
	struct DESC : oGPU_BUFFER_DESC {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUMesh : oGPUResource
{
	// Ranges of triangles are grouped, but kept separate from one another so that 
	// a continuous shape can be constructed by multiple draw calls, each with a 
	// different render state.

	struct DESC : oGPU_MESH_DESC {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUTexture : oGPUResource
{
	// A large buffer filled with surface data. Most often this is one or more 
	// 2D planes wrapped onto the surface of a screen or mesh.

	struct DESC : oGPU_TEXTURE_DESC {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPURenderTarget : oGPUDeviceChild
{
	// A 2D plane onto which rendering occurs

	struct DESC : oGPU_RENDER_TARGET_DESC {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Modifies the values for clearing without modifying other topology
	virtual void SetClearDesc(const oGPU_CLEAR_DESC& _ClearDesc) threadsafe = 0;

	// Resizes all buffers without changing formats or other topology
	virtual void Resize(const int2& _NewDimensions) = 0;

	// Accesses a readable texture for the specified render target in an MRT.
	virtual void GetTexture(int _MRTIndex, oGPUTexture** _ppTexture) = 0;

	// Accesses a readable texture for the depth-stencil buffer. This can fail if 
	// there is no depth-stencil buffer or if the buffer is of a non-readable 
	// format.
	virtual void GetDepthTexture(oGPUTexture** _ppTexture) = 0;
};

interface oGPUPipeline : oGPUDeviceChild
{
	// A pipeline is the result of setting all stages of the programmable pipeline 
	// (all shaders) and the vertex input format to that pipeline.

	struct DESC : oGPU_PIPELINE_BYTECODE {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUComputeShader : oGPUDeviceChild
{
	// That other pipeline moder GPUs support. This is the CUDA/Compute/OpenCL 
	// path that ignores fixed-function rasterization stages and exposes more 
	// general-purpose components.

	struct DESC : oGPU_COMPUTE_SHADER_BYTECODE {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;
};

interface oGPUCommandList : oGPUDeviceChild
{
	// A container for a list of commands issued by the user to the graphics 
	// device. All operations herein are single-threaded. For parallelism separate 
	// command lists can be built in different threads.

	struct DESC : oGPU_COMMAND_LIST_DESC {};
	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Begins recording of GPU command submissions. All rendering for this context 
	// should occur between Begin() and End().
	virtual void Begin() = 0;

	// Ends recording of GPU submissions and caches a command list
	virtual void End() = 0;

	// Allocates internal device memory that can be written to (not read) and 
	// committed to the device to update the specified resource.
	virtual void Reserve(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource) = 0;

	// Commits memory to the specified resource. If the memory in _Source.pData 
	// was reserved, then this will free the memory. If _Source.pData is user 
	// memory, it will not be freed. NOTE: If _Source.pData was reserved, then 
	// _Subregion MUST be empty, since Reserve does not allocate subregions, only 
	// whole subresources. When using A subregion can be updated using _Subregion. 
	// If the specified rectangle is empty on any dimension, then the entire 
	// surface will be copied (default behavior). If the item is a 1D structure 
	// then Min.x is the offset and Max.x is offset + count. and Min.y should be 0 
	// and Max.y should be 1. If a 2D resource, then dimensions make sense. 3D 
	// resources are treated as slices of a 2D resources, so 3D resources behave 
	// the same as 2D resources for any given subresource identifier. For 
	// oGPUInstanceLists and oGPULineLists _Subregion represents the valid portion 
	// AFTER update in its Min.x and Max.x values, i.e. Min.x is where to begin 
	// drawing and Max.x is one after the last valid element to draw. So for an 
	// oGPUInstanceList allocated with MaxNumInstances = 100, updating for 10 to 
	// be drawn would use oRECT(int2(0,0), int2(10,1)).
	virtual void Commit(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Source, const oRECT& _Subregion = oRECT()) = 0;

	// Copies the contents from one resource to another. Both must have compatible 
	// (often identical) topologies. A common use of this API is to copy from a 
	// source resource to a readback copy of the same resource so it can be 
	// accessed from the CPU.
	virtual void Copy(oGPUResource* _pDestination, oGPUResource* _pSource) = 0;

	// Set the texture sampler states in this context
	virtual void SetSamplers(int _StartSlot, int _NumStates, const oGPU_SAMPLER_STATE* _pSamplerState) = 0;

	// Set the textures in this context
	virtual void SetTextures(int _StartSlot, int _NumTextures, const oGPUTexture* const* _ppTextures) = 0;

	// Set the constants in this context
	virtual void SetConstants(int _StartSlot, int _NumConstants, const oGPUBuffer* const* _ppBuffers) = 0;

	// _____________________________________________________________________________
	// Rasterization-specific

	// Sets the render target to which rendering will occur. By default, a single
	// full-target viewport is created, else it can be overridden. A viewport is 
	// a 3D box whose minimum is at the top, left, near corner of the viewable 
	// frustum, and whose maximum is at the bottom, right, far corner of the 
	// viewable frustum. A full-target viewport would most often be: 
	// oAABoxf(float3(0.0f), float3(RTWidth, RTHeight, 1.0f))
	virtual void SetRenderTarget(oGPURenderTarget* _pRenderTarget, int _NumViewports = 0, const oAABoxf* _pViewports = nullptr) = 0;

	// Same as SetRenderTarget, but also sets up unordered textures: side-effect
	// random access rendertargets/sample-from textures whose atomicity of access
	// must explicitly be handled by the shader, not by the scheduling of the GPU.
	virtual void SetRenderTargetAndUnorderedTextures(oGPURenderTarget* _pRenderTarget, int _NumViewports, const oAABoxf* _pViewports, int _NumUnorderedTextures, oGPUTexture** _ppUnorderedTextures) = 0;

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

	// Submits an oGPUMesh for drawing using the current state of the 
	// command list.
	virtual void Draw(const oGPUMesh* _pMesh, int _RangeIndex, const oGPUInstanceList* _pInstanceList = nullptr) = 0;

	// Draws a set of worldspace lines. Use Map/Unmap to set up line lists
	// writing an array of type oGPULineList::LINEs.
	virtual void Draw(const oGPULineList* _pLineList) = 0;
	
	virtual void DrawSVQuad(uint _NumInstances = 1) = 0;

	// _____________________________________________________________________________
	// Compute-specific

	// Clears the specified resource created for unordered access.
	virtual void ClearI(oGPUResource* _pUnorderedResource, const uint _Values[4]) = 0;

	// Clears the specified resource created for unordered access.
	virtual void ClearF(oGPUResource* _pUnorderedResource, const float _Values[4]) = 0;

	// If all 3 values in _ThreadGroupCount are oInvalid, then the counts are 
	// automatically determined by the GPU. Unordered resources can be 
	// oGPUBuffer or oGPUTexture, but each must have been created readied
	// for unordered access.
	virtual void Dispatch(oGPUComputeShader* _pComputeShader, const int3& _ThreadGroupCount, int _NumUnorderedResources, const oGPUResource* const* _ppUnorderedResources) = 0;

	// Same as Dispatch above, but it gets its thread group count from oGPUBuffer.
	virtual void Dispatch(oGPUComputeShader* _pComputeShader, oGPUBuffer* _pThreadGroupCountBuffer, int _AlignedByteOffsetToThreadGroupCount, int _NumUnorderedResources, const oGPUResource* const* _ppUnorderedResources) = 0;
};

interface oGPUDevice : oInterface
{
	// Main SW abstraction for a graphics processor

	struct INIT : oGPU_DEVICE_INIT { INIT(const char* _DebugName = "oGPUDevice") : oGPU_DEVICE_INIT(_DebugName) {} };
	struct DESC : oGPU_DEVICE_DESC {};

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

	virtual bool CreateCommandList(const char* _Name, const oGPUCommandList::DESC& _Desc, oGPUCommandList** _ppCommandList) = 0;
	virtual bool CreateLineList(const char* _Name, const oGPULineList::DESC& _Desc, oGPULineList** _ppLineList) = 0;
	virtual bool CreatePipeline(const char* _Name, const oGPUPipeline::DESC& _Desc, oGPUPipeline** _ppPipeline) = 0;
	virtual bool CreateComputeShader(const char* _Name, const oGPUComputeShader::DESC& _Desc, oGPUComputeShader** _ppComputeShader) = 0;
	virtual bool CreateRenderTarget(const char* _Name, const oGPURenderTarget::DESC& _Desc, oGPURenderTarget** _ppRenderTarget) = 0;
	virtual bool CreateBuffer(const char* _Name, const oGPUBuffer::DESC& _Desc, oGPUBuffer** _ppBuffer) = 0;
	virtual bool CreateMesh(const char* _Name, const oGPUMesh::DESC& _Desc, oGPUMesh** _ppMesh) = 0;
	virtual bool CreateInstanceList(const char* _Name, const oGPUInstanceList::DESC& _Desc, oGPUInstanceList** _ppInstanceList) = 0;
	virtual bool CreateTexture(const char* _Name, const oGPUTexture::DESC& _Desc, oGPUTexture** _ppTexture) = 0;

	// MapRead is a non-blocking call to read from the specified resource by the
	// mapped data populated in the specified _pMappedSubresource. If the function
	// would block, this returns false. If it succeeds, call ReadEnd to unlock.
	// the buffer. This will also return false for resources not of type READBACK.
	virtual bool MapRead(oGPUResource* _pReadbackResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource, bool _bBlocking=false) = 0;
	virtual void UnmapRead(oGPUResource* _pReadbackResource, int _Subresource) = 0;

	virtual bool BeginFrame() = 0;
	virtual void EndFrame() = 0;
};

oAPI bool oGPUDeviceCreate(const oGPUDevice::INIT& _Init, oGPUDevice** _ppDevice);

#endif
