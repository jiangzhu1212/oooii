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
// common code for a manager to hold both hard-coded and loaded shaders.
// initializes are protected because the intent is to have derived typed
// instances of this class and hard-coded shaders set up in initialize 
// functions defined there, but use the basic setup functions defined here.
#ifndef oGfx_shader_registry_h
#define oGfx_shader_registry_h

#include <oBase/allocate.h>
#include <oBase/concurrent_registry.h>
#include <oBase/object_pool.h>
#include <oGPU/shader.h>
#include <oGPU/device.h>

namespace ouro {
	namespace gfx {

template<typename shader_t>
class shader_registry
{
public:
	typedef shader_t shader_type;

	typedef concurrent_registry::index_type size_type;
	typedef concurrent_registry::index_type index_type;
	
	static const gpu::stage::value stage = shader_type::stage;

	shader_registry() : d(nullptr) {}
	~shader_registry() { deinitialize(); }

	void* deinitialize()
	{
		if (d)
		{
			r.unmake_all();
			r.flush();
			d = nullptr;
			r.deinitialize();
			return s.deinitialize();
		}

		return nullptr;
	}

	void make(const char* name, scoped_allocation& bytecode, const path& path, bool force = false)
	{
		r.make(name, bytecode, path, force);
	}

	void unmake(const char* name)
	{
		r.unmake(name);
	}

	void compile(const lstring& include_paths, const lstring& defines, const path& source_path, const char* shader_source, const char* entry_point)
	{
		try
		{
			scoped_allocation bytecode = gpu::compile_shader(include_paths, defines, source_path, stage, entry_point, shader_source);
			oTRACEA("[%s registry] insert \"%s\" from %s", as_string(stage), entry_point, source_path.c_str());
			r.make(entry_point, bytecode, source_path, true);
		}
			
		catch (std::exception& e)
		{
			oTRACEA("[%s registry] insert \"%s\" as error", as_string(stage), entry_point, e.what());
			scoped_allocation empty;
			r.make(entry_point, empty, source_path, true);
		}
	}

protected:

	size_type initialize(void* memory, size_type capacity, gpu::device& dev, const void* missing_bytecode, const void* failed_bytecode, const void* making_bytecode)
	{
		scoped_allocation missing((void*)missing_bytecode, 1, noop_deallocate);
		scoped_allocation failed((void*)failed_bytecode, 1, noop_deallocate);
		scoped_allocation making((void*)making_bytecode, 1, noop_deallocate);

		concurrent_registry::lifetime_t lifetime;
		lifetime.create = [&](scoped_allocation& compiled, const char* name)->void* { return s.create<const char*, gpu::device&, scoped_allocation&>(name, *d, compiled); };
		lifetime.destroy = [&](void* entry) { s.destroy((shader_type*)entry); };

		concurrent_registry::placeholder_source_t placeholders;
		placeholders.compiled_missing = std::move(missing);
		placeholders.compiled_failed = std::move(failed);
		placeholders.compiled_making = std::move(making);

		size_type ssize = s.initialize(nullptr, capacity);
		size_type aligned_ssize = byte_align(ssize, sizeof(void*));
		size_type rsize = r.initialize(nullptr, capacity, lifetime, placeholders);
		size_type req = aligned_ssize + rsize;

		if (memory)
		{
			d = &dev;
			s.initialize(memory, capacity);
			memory = byte_add(memory, aligned_ssize);
			r.initialize(memory, capacity, lifetime, placeholders);
		}

		return req;
	}

	size_type initialize(size_type capacity, gpu::device& dev, const void* missing_bytecode, const void* failed_bytecode, const void* making_bytecode)
	{
		size_type req = initialize(nullptr, capacity, dev, missing_bytecode, failed_bytecode, making_bytecode);
		if (!req)
			return 0;

		void* p = default_allocate(req, 0);

		return initialize(p, capacity, dev, missing_bytecode, failed_bytecode, making_bytecode);
	}

	gpu::device* d;
	concurrent_registry r;
	object_pool<shader_type> s;
};

	} // namespace gfx
} // namespace ouro

#endif
