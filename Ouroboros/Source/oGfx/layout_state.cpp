// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGfx/layout_state.h>

namespace ouro { namespace gfx {

void layout_state::initialize(const char* name, gpu::device& dev)
{
	deinitialize();
	mstring n;
	for (int i = 0; i < gpu::intrinsic::vertex_layout::count; i++)
	{
		gpu::intrinsic::vertex_layout::value input = gpu::intrinsic::vertex_layout::value(i);
		snprintf(n, "%s vertex_layout::%s", name, as_string(input));
		layouts[i].initialize(n, dev, gpu::intrinsic::elements(input), gpu::intrinsic::vs_byte_code(input));
	}
}

void layout_state::deinitialize()
{
	for (auto& layout : layouts)
		layout.deinitialize();
}

}}
