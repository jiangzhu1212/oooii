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
#include <oGfx/oGfxShaderRegistry.h>
#include <oGfx/oGfxShaders.h>
#include <oCore/process_heap.h>

namespace ouro {
	namespace gfx {
#if 0
class shader_context
{
public:
	static shader_context& singleton();

	shader_context();
	~shader_context();

private:
	concurrent_registry Shaders[gpu::shader_type::count];
};

oDEFINE_PROCESS_SINGLETON("ouro::gfx::shader_context", shader_context);

shader_context::shader_context()
{
	gpu::device* _pDevice = nullptr;
	static const uint kExpectedCounts[gpu::shader_type::count] = { 20, 5, 5, 10, 100, 100 };
	uint SizeBytes[gpu::shader_type::count];

	size_t Total = 0;
	for (uint i : kExpectedCounts)
	{
		SizeBytes[i] = byte_align(concurrent_registry::calc_size(kExpectedCounts[i]), oDEFAULT_MEMORY_ALIGNMENT);
		Total += SizeBytes[i];
	}

	void* p = default_allocator.allocate(Total, 0);

	// Pixel Shaders
	{
		scoped_allocation missing((void*)byte_code(pixel_shader::yellow), 1, noop_deallocate);
		scoped_allocation failed((void*)byte_code(pixel_shader::red), 1, noop_deallocate);
		scoped_allocation making((void*)byte_code(pixel_shader::white), 1, noop_deallocate);

		Shaders[gpu::shader_type::pixel] = std::move(concurrent_registry(p
			, kExpectedCounts[gpu::shader_type::pixel]
			, [=](scoped_allocation& _Compiled, const char* _Name)->void* { return (void*)gpu::make_shader(_pDevice, gpu::shader_type::pixel, _Compiled, _Name); }
			, [=](void* _pEntry) { gpu::unmake_shader((gpu::shader)_pEntry); }
			, missing
			, failed
			, making));
		
		p = byte_align(byte_add(p, SizeBytes[gpu::shader_type::pixel]), oDEFAULT_MEMORY_ALIGNMENT);
	}
}

shader_context::~shader_context()
{
}
#endif
	} // namespace gfx
} // namespace ouro
