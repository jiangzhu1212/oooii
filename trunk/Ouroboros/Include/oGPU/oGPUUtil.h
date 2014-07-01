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
// Utility code that uses oGPU to implement extended functionality.
#pragma once
#ifndef oGPUUtil_h
#define oGPUUtil_h

#include <oGPU/oGPU.h>

#include <oMesh/obj.h>

namespace ouro {
	namespace gpu {

namespace oGPUUtilLayout
{ oDECLARE_SMALL_ENUM(value, uchar) {

	// positions: float3
	// normals: dec3n
	// tangents: dec3n
	// texcoords0: half2 \ mutually exclusive
	// texcoords0: half4 /
	// texcoords1: half2 \ mutually exclusive
	// texcoords1: half4 /
	// colors: color

	// if this is updated, remember to update is_positions, etc.

	none,

	pos,
	color,
	pos_color,
	pos_nrm,
	pos_nrm_tan,
	pos_nrm_tan_uv0,
	pos_nrm_tan_uvwx0,
	pos_uv0,
	pos_uvwx0,
	uv0,
	uvwx0,
	uv0_color,
	uvwx0_color,

	count,

};}

struct oGPUUtilSource // @tony temp move from mesh.h (should be removed altogether) during refactor)
{
	// this struct is a general-purpose container for vertex source data.
	// the intent is not that every pointer be populated but rather if one
	// is populated than it contains typed data with the specified stride.

	oGPUUtilSource()
		: indicesi(nullptr)
		, indicess(nullptr)
		, ranges(nullptr)
		, positionsf(nullptr)
		, normalsf(nullptr)
		, normals(nullptr)
		, tangentsf(nullptr)
		, tangents(nullptr)
		, uv0sf(nullptr)
		, uvw0sf(nullptr)
		, uvwx0sf(nullptr)
		, uv0s(nullptr)
		, uvwx0s(nullptr)
		, colors(nullptr)
		, indexi_pitch(0)
		, indexs_pitch(0)
		, range_pitch(0)
		, positionf_pitch(0)
		, normalf_pitch(0)
		, normal_pitch(0)
		, tangentf_pitch(0)
		, tangent_pitch(0)
		, uv0f_pitch(0)
		, uvw0f_pitch(0)
		, uvwx0f_pitch(0)
		, uv0_pitch(0)
		, uvwx0_pitch(0)
		, color_pitch(0)
		, vertex_layout(oGPUUtilLayout::none)
	{}

	const uint* indicesi;
	const ushort* indicess;

	const mesh::range* ranges;
	
	const float3* positionsf;
	const float3* normalsf;
	const dec3n* normals;
	const float4* tangentsf;
	const dec3n* tangents;
	const float2* uv0sf;
	const float3* uvw0sf;
	const float4* uvwx0sf;
	const half2* uv0s;
	const half4* uvwx0s;
	const color* colors;

	uint indexi_pitch;
	uint indexs_pitch;
	uint range_pitch;
	uint positionf_pitch;
	uint normalf_pitch;
	uint normal_pitch;
	uint tangentf_pitch;
	uint tangent_pitch;
	uint uv0f_pitch;
	uint uvw0f_pitch;
	uint uvwx0f_pitch;
	uint uv0_pitch;
	uint uvwx0_pitch;
	uint color_pitch;

	oGPUUtilLayout::value vertex_layout;

	inline bool operator==(const oGPUUtilSource& _That) const
	{
		const void* const* thisP = (const void* const*)&indicesi;
		const void* const* end = (const void* const*)&colors;
		const void* const* thatP = (const void* const*)&_That.indicesi;
		while (thisP <= end) if (*thisP++ != *thatP++) return false;
		const uint* thisI = &indexi_pitch;
		const uint* endI = &color_pitch;
		const uint* thatI = &_That.indexi_pitch;
		while (thisI <= endI) if (*thisI++ != *thatI++) return false;
		return vertex_layout == _That.vertex_layout;
	}
	inline bool operator!=(const oGPUUtilSource& _That) const { return !(*this == _That); }
};

uint CalcVertexSize(const oGPUUtilLayout::value& _Layout);
oGPUUtilLayout::value CalcLayout(const oGPUUtilSource& _Source);

ouro::mesh::element_array get_elements(const oGPUUtilLayout::value& _Layout);

// _____________________________________________________________________________
// Buffer convenience functions

// Commit the contents of regular memory to device memory. Mainly this is 
// intended to be used with the templated types below.
void commit_buffer(command_list* _pCommandList, buffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs);
void commit_buffer(device* _pDevice, buffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs);
template<typename T> void commit_buffer(command_list* _pCommandList, buffer* _pBuffer, const T& _Struct) { commit_buffer(_pCommandList, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void commit_buffer(command_list* _pCommandList, buffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { commit_buffer(_pCommandList, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }
template<typename T> void commit_buffer(device* _pDevice, buffer* _pBuffer, const T& _Struct) { commit_buffer(_pDevice, _pBuffer, &_Struct, sizeof(_Struct), 1); }
template<typename T> void commit_buffer(device* _pDevice, buffer* _pBuffer, const T* _pStructArray, uint _NumStructs) { commit_buffer(_pDevice, _pBuffer, _pStructArray, sizeof(T), _NumStructs); }

// Commit _MappedSubresource to _pIndexBuffer handling 16-bit to 32-bit and vice 
// versa converstions between the source and destination.
void commit_index_buffer(command_list* _pCommandList
	, const surface::const_mapped_subresource& _MappedSubresource
	, buffer* _pIndexBuffer);

inline void commit_index_buffer(device* _pDevice
	, const surface::const_mapped_subresource& _MappedSubresource
	, buffer* _pIndexBuffer)
{
	commit_index_buffer(_pDevice->immediate(), _MappedSubresource, _pIndexBuffer);
}

inline void commit_index_buffer(device* _pDevice
	, const surface::const_mapped_subresource& _MappedSubresource
	, std::shared_ptr<buffer>& _pIndexBuffer)
{
	commit_index_buffer(_pDevice->immediate(), _MappedSubresource, _pIndexBuffer.get());
}

// Client code must define a function that given a vertex trait returns a mapping
// of source data. If the function returns a mapping with a null data pointer,
// values for that vertex trait will be set to 0 in _pVertexBuffer. No conversions
// will be done in this function so if a trait is asked for in a compressed format
// the returned mapping is expected to be in that compressed format.
void commit_vertex_buffer(command_list* _pCommandList, const oGPUUtilLayout::value& _Layout, const oGPUUtilSource& _Source, buffer* _pVertexBuffer);
inline void commit_vertex_buffer(device* _pDevice, const oGPUUtilLayout::value& _Layout, const oGPUUtilSource& _Source, buffer* _pVertexBuffer)
{
	commit_vertex_buffer(_pDevice->immediate(), _Layout, _Source, _pVertexBuffer);
}

inline void commit_vertex_buffer(device* _pDevice, const oGPUUtilLayout::value& _Layout, const oGPUUtilSource& _Source, std::shared_ptr<buffer>& _pVertexBuffer)
{
	commit_vertex_buffer(_pDevice->immediate(), _Layout, _Source, _pVertexBuffer.get());
}

// Create an index buffer based on the parameters. If _MappedSubresource.pData
// is not null commit_index_buffer() is called on that data. If it is null then 
// only the creation is done.
std::shared_ptr<buffer> make_index_buffer(device* _pDevice, const char* _Name, uint _NumIndices, uint _NumVertices
	, const surface::const_mapped_subresource& _MappedSubresource = surface::const_mapped_subresource());

inline std::shared_ptr<buffer> make_index_buffer(std::shared_ptr<device>& _pDevice, const char* _Name, uint _NumIndices, uint _NumVertices
	, const surface::const_mapped_subresource& _MappedSubresource = surface::const_mapped_subresource())
{ return make_index_buffer(_pDevice.get(), _Name, _NumIndices, _NumVertices, _MappedSubresource); }

// Creates a vertex buffer based on the parameters. If _GetElementData is valid 
// then commit_vertex_buffer is called.
std::shared_ptr<buffer> make_vertex_buffer(device* _pDevice, const char* _Name, const oGPUUtilLayout::value& _Layout
	, uint _NumVertices, const oGPUUtilSource& _Source = oGPUUtilSource());

inline std::shared_ptr<buffer> make_vertex_buffer(std::shared_ptr<device>& _pDevice, const char* _Name, const oGPUUtilLayout::value& _Layout
	, uint _NumVertices, const oGPUUtilSource& _Source = oGPUUtilSource())
{ return make_vertex_buffer(_pDevice.get(), _Name, _Layout, _NumVertices, _Source); }

// Creates a readback buffer or texture sized to be able to completely contain the 
// specified source.
std::shared_ptr<buffer> make_readback_copy(buffer* _pSource);
std::shared_ptr<texture> make_readback_copy(texture* _pSource);

// Optionally allocates a new buffer and reads the counter from the specified 
// buffer into it. For performance a buffer can be specified to receive the 
// counter value. The value is the read back to a uint using the immediate 
// command list. The purpose of this utility code is primarily to wrap the 
// lengthy code it takes to get the counter out for debugging/inspection 
// purposes and should not be used in production code since it synchronizes/
// stalls the device. This returns ouro::invalid on failure. REMEMBER THAT THIS MUST 
// BE CALLED AFTER A FLUSH OF ALL COMMANDLISTS THAT WOULD UPDATE THE COUNTER. 
// i.e. the entire app should use the immediate command list, otherwise this 
// could be sampling stale data. If using the immediate command list everywhere 
// is not an option, this must be called after Device::end_frame() to have valid 
// values.
uint read_back_counter(buffer* _pUnorderedBuffer, buffer* _pPreallocatedReadbackBuffer = nullptr);

// Reads the source resource into the memory pointed at in the destination struct. 
// Since this is often used for textures flip vertically can do an in-place/
// during-copy flip. This returns the result of map_read. 
bool read(resource* _pSourceResource, int _Subresource, ouro::surface::mapped_subresource& _Destination, bool _FlipVertically = false);

// _____________________________________________________________________________
// Texture convenience functions

// Creates a texture and fills it with source image data according to the type specified. 
// NOTE: render target and readback not tested.
std::shared_ptr<texture> make_texture(device* _pDevice, const char* _Name, const surface::buffer* const* _ppSourceImages, uint _NumImages, texture_type::value _Type);
inline std::shared_ptr<texture> make_texture(std::shared_ptr<device>& _pDevice, const char* _Name, const surface::buffer* const* _ppSourceImages, uint _NumImages, texture_type::value _Type) { return make_texture(_pDevice.get(), _Name, _ppSourceImages, _NumImages, _Type); }

// Creates a surface buffer and copies the specified subresource to it. If _Subresource is
// invalid all subresources are copied.
std::shared_ptr<surface::buffer> copy_to_surface_buffer(texture* _pSource, int _Subresource = invalid);

	} // namespace gpu
} // namespace ouro

#if 0

// Binds the readable (samplable) shader resources from a render target in order
inline void oGPURenderTargetSetShaderResources(command_list* _pCommandList, int _StartSlot, bool _IncludeDepthStencil, oGPURenderTarget* _pRenderTarget)
{
	oGPURenderTarget::DESC rtd;
	_pRenderTarget->GetDesc(&rtd);
	ouro::std::shared_ptr<texture> MRTs[ouro::gpu::max_num_mrts+1];
	int i = 0;
	for (; i < rtd.mrt_count; i++)
		_pRenderTarget->GetTexture(i, &MRTs[i]);
	if (_IncludeDepthStencil)
		_pRenderTarget->GetDepthTexture(&MRTs[i++]);
	_pCommandList->SetShaderResources(_StartSlot, i, (const resource* const*)MRTs);
}

// _____________________________________________________________________________
// Mesh convenience functions

bool oGPUReadVertexSource(int _Slot, int _NumVertices, ouro::surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const oGeometry::DESC& _Desc, oGeometry::CONST_MAPPED& _GeoMapped);
bool oGPUReadVertexSource(int _Slot, int _NumVertices, ouro::surface::mapped_subresource& _Mapped, uint _NumElements, const oGPU_VERTEX_ELEMENT* _pElements, const threadsafe oOBJ* _pOBJ);

#endif
#endif