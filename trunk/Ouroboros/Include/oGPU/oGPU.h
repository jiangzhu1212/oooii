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
// remain policy-agnostic (i.e. this is not a renderer, just a hardware abstraction).
#pragma once
#ifndef oGPU_h
#define oGPU_h

#include <oBase/allocate.h>
#include <oBase/dec3n.h>
#include <oBase/gpu_api.h>
#include <oBase/invalid.h>
#include <oBase/macros.h>
#include <oBase/types.h>
#include <oBase/vendor.h>
#include <oBase/version.h>
#include <oMesh/mesh.h>
#include <oSurface/buffer.h>
#include <array>

#include <oGPU/device.h>
#include <oGPU/shader.h>
#include <oGPU/state.h>
#include <oGPU/vertex_layouts.h>

#include <oGUI/oGUI.h> // only for ouro::draw_context_handle... can it be abstracted

namespace ouro {
	namespace gpu {

// _____________________________________________________________________________
// Device concepts

// Main SW abstraction for a graphics processor. The class is defined below since it uses
// all other objects defined in this header.
class device;

// Main SW abstraction for a single thread of graphics command preparation. This object
// is defined below since it uses all other objecsts defined in this header.
class command_list;

class device_child
{
	// Anything allocated from the device is a device child
public:
	// fill the specified pointer with this resources's associated device. The 
	// device's ref count is incremented.
	virtual std::shared_ptr<device> get_device() const = 0;

	// Returns the identifier as specified at create time.
	virtual const char* name() const = 0;
};

// _____________________________________________________________________________
// Pipeline concepts

struct basic_pipeline1_info
{
	const char* debug_name;
	mesh::element_array vertex_layout;
	mesh::primitive_type::value primitive_type;
	const void* vs, *hs, *ds, *gs, *ps;
};

struct pipeline1_info : public basic_pipeline1_info
{
	pipeline1_info()
	{
		debug_name = "unnamed pipeline";
		vertex_layout.fill(mesh::element());
		primitive_type = mesh::primitive_type::unknown;
		vs = nullptr; hs = nullptr; ds = nullptr; gs = nullptr; ps = nullptr;
	}
};

class pipeline1 : public device_child 
{
	// A pipeline is the result of setting all stages of the programmable pipeline 
	// (all shaders) and the vertex input format to that pipeline.
public:
	virtual pipeline1_info get_info() const = 0;
};

// _____________________________________________________________________________
// Execution concepts

struct command_list_info
{
	static const short immediate = -1;

	short draw_order;
};

class command_list : public device_child 
{
	// A container for a list of commands issued by the user to the graphics 
	// device. All operations herein are single-threaded. For parallelism separate 
	// command lists can be built in different threads.
public:
	virtual command_list_info get_info() const = 0;

	// Begins/ends recording of GPU command submissions. All rendering for this context 
	// should occur between Begin() and End().
	virtual void begin() = 0;
	virtual void end() = 0;

	// This should never be required to be called, and has bad performance 
	// implications if called but is sometimes required, especially with the 
	// immediate context during debugging.
	virtual void flush() = 0;

	// Clears the command list's idea of state back to its default. In general 
	// client code should set state absolute state, but if a command list is 
	// reassigned to a different responsibility, it may make sense to start with
	// a clean slate.
	virtual void reset() = 0;

	// Set the texture sampler states in this context
	virtual void set_samplers(uint _StartSlot, uint _NumStates, const sampler_state::value* _pSamplerState) = 0;
	inline void set_sampler(uint _StartSlot, const sampler_state::value& _SamplerState) { set_samplers(_StartSlot, 1, &_SamplerState); }

	virtual void set_pipeline(const pipeline1* _pPipeline) = 0;
	inline void set_pipeline(const std::shared_ptr<pipeline1>& _pPipeline) { set_pipeline(_pPipeline.get()); }

	// Set the rasterization state in this context
	virtual void set_rasterizer_state(const rasterizer_state::value& _State) = 0;

	// Set the output merger (blend) state in this context
	virtual void set_blend_state(const blend_state::value& _State) = 0;

	// Set the depth-stencil state in this context
	virtual void set_depth_stencil_state(const depth_stencil_state::value& _State) = 0;
};

class device
{
	// Main SW abstraction for a graphics processor
public:
	static std::shared_ptr<device> make(const device_init& _Init);

	virtual device_info get_info() const = 0;
	
	virtual const char* name() const = 0;
	virtual int frame_id() const = 0;

	// Returns a command list with an invalid DrawOrder in its DESC that does not
	// lock or wait for begin_frame/end_frame. Be very careful with this because it
	// sets state that deferred command lists may not be aware of and does so
	// immediately without care for ordering. This should only be called from a 
	// single thread. Calls to this CommandList need not be inside a begin_frame/
	// end_frame. Begin()/End() for the immediate command list will noop, so either
	// they don't need to be called or an immediate command list should be able to 
	// substitute for a regular (deferred) command list without code change 
	// (though timing and order differences are likely).
	virtual std::shared_ptr<command_list> get_immediate_command_list() = 0;

	virtual command_list* immediate() = 0;

	virtual std::shared_ptr<command_list> make_command_list(const char* _Name, const command_list_info& _Info) = 0;
	virtual std::shared_ptr<pipeline1> make_pipeline1(const char* _Name, const pipeline1_info& _Info) = 0;

	// convenience versions of the above
	inline std::shared_ptr<command_list> make_command_list(const char* _Name, short _DrawOrder = 0) { command_list_info i; i.draw_order = _DrawOrder; return make_command_list(_Name, i); } 
	inline std::shared_ptr<pipeline1> make_pipeline1(const pipeline1_info& _Info) { return make_pipeline1(_Info.debug_name, _Info); }

	virtual bool begin_frame() = 0;
	virtual void end_frame() = 0;
};

class scoped_device_frame
{
	device* dev;
public:
	scoped_device_frame(device* dev) : dev(dev) { dev->begin_frame(); }
	~scoped_device_frame() { if (dev) dev->end_frame(); }
};

class scoped_command_line_frame
{
	command_list* cl;
public:
	scoped_command_line_frame(command_list* cl) : cl(cl) { cl->begin(); }
	~scoped_command_line_frame() { cl->end(); }
};


	} // namespace gpu
} // namespace ouro

#if 0

#include <oBasis/oBuffer.h>
#include <oSurface/surface.h>
#include <oGUI/window.h>

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
bool oGPUSurfaceConvert(
	void* oRESTRICT _pDestination
	, uint _DestinationRowPitch
	, surface::format _DestinationFormat
	, const void* oRESTRICT _pSource
	, uint _SourceRowPitch
	, surface::format _SourceFormat
	, const int2& _MipDimensions);

// Extract the parameters for the above call directly from textures
bool oGPUSurfaceConvert(oGPUTexture* _pSourceTexture, surface::format _NewFormat, oGPUTexture** _ppDestinationTexture);

// Loads a texture from disk. The _Desc specifies certain conversions/resizes
// that can occur on load. Use 0 or surface::unknown to use values as 
// they are found in the specified image resource1/buffer.
// @tony: At this time the implementation does NOT use oImage loading 
// code plus a simple call to call oGPUCreateTexture(). Because this API 
// supports conversion for any surface::format, at this time we defer to 
// DirectX's .dds support for advanced formats like BC6 and BC7 as well as their
// internal conversion library. When it's time to go cross-platform, we'll 
// revisit this and hopefully call more generic code.
bool oGPUTextureLoad(oGPUDevice* _pDevice, const texture_info& _Info, const char* _URIReference, const char* _DebugName, oGPUTexture** _ppTexture);
bool oGPUTextureLoad(oGPUDevice* _pDevice, const texture_info& _Info, const char* _DebugName, const void* _pBuffer, uint _SizeofBuffer, oGPUTexture** _ppTexture);

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
bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, void* _pBuffer, uint _SizeofBuffer);
bool oGPUTextureSave(oGPUTexture* _pTexture, oGPU_FILE_FORMAT _Format, const char* _Path);

#endif
#endif