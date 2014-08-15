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
#include <oBase/allocate.h>
#include <oBase/macros.h>
#include <oBase/assert.h>

namespace ouro {

void* default_allocate(size_t _Size, unsigned int options)
{
	allocate_options opt; opt.options = options;
	void* p = _aligned_malloc(_Size, __max(memory_alignment::align_to_default, 1 << opt.alignment));
	oTRACE("Alloc %p", p);
	return p;
}

void default_deallocate(const void* pointer)
{
	oTRACE("Free %p", pointer);
	_aligned_free((void*)pointer);
}

void* noop_allocate(size_t _Size, unsigned int _Options)
{
	return nullptr;
}

void noop_deallocate(const void* pointer)
{
}

allocator default_allocator(default_allocate, default_deallocate);
allocator noop_allocator(noop_allocate, noop_deallocate);
	
const char* as_string(const memory_alignment::value& alignment)
{
	static const char* names[] = 
	{
		"align_to_1",
		"align_to_2",
		"align_to_4",
		"align_to_8",
		"align_to_16 (default)",
		"align_to_32",
		"align_to_64 (cacheline)",
		"align_to_128",
		"align_to_256",
		"align_to_512",
		"align_to_1024",
		"align_to_2048",
		"align_to_4096",
	};
	static_assert(oCOUNTOF(names) == memory_alignment::count, "array mismatch");
	return names[alignment];
}

const char* as_string(const memory_type::value& type)
{
	static const char* names[] = 
	{
		"default_memory",
		"io_read_write",
		"cpu_writecombine",
		"cpu_gpu_coherent",
		"gpu_writecombine",
		"gpu_readonly",
		"gpu_on_chip",
		"physical",
	};
	static_assert(oCOUNTOF(names) == memory_type::count, "array mismatch");
	return names[type];
}

const char* as_string(const subsystem::value& subsystem)
{
	static const char* names[] = 
	{
		"ai",
		"app",
		"bookkeeping",
		"cpu",
		"input",
		"io",
		"graphics",
		"physics",
		"sound",
		"ui",
	};
	static_assert(oCOUNTOF(names) == subsystem::count, "array mismatch");
	return names[subsystem];
}

} // namespace ouro
