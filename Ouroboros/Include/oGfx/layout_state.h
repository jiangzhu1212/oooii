// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Manages vertex_layouts so they can be accessed by enum.
#ifndef oGfx_layout_state_h
#define oGfx_layout_state_h

#include <oGPU/shaders.h>
#include <oGPU/vertex_layout.h>

namespace ouro { namespace gfx {

class layout_state
{
public:
	layout_state() {}
	~layout_state() { deinitialize(); }

	void initialize(const char* name, gpu::device& dev);
	void deinitialize();

	inline void set(gpu::command_list& cl, const gpu::intrinsic::vertex_layout::value& input, const mesh::primitive_type& prim_type) const { layouts[input].set(cl, prim_type); }

private:
	std::array<gpu::vertex_layout, gpu::intrinsic::vertex_layout::count> layouts;
};

}}

#endif
