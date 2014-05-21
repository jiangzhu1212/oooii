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
#pragma once
#ifndef oGPU_resource_h
#define oGPU_resource_h

#include <oBase/allocate.h>
#include <oBase/macros.h>
#include <oBase/types.h>
#include <oSurface/surface.h>

namespace ouro {
	namespace gpu {

static const uint max_num_resources = 128;

namespace cube_face
{ oDECLARE_SMALL_ENUM(value, uchar) {

	posx,
	negx,
	posy,
	negy,
	posz,
	negz,

	count,

};}

namespace resource_type
{ oDECLARE_SMALL_ENUM(value, uchar) {
	
	unknown,
	constant_buffer,
	index_buffer,
	vertex_buffer,
	raw_buffer,
	unstructured_buffer,
	structured_buffer,
	structured_append, 
	structured_counter,
	texturecube,
	texture1d,
	texture2d,
	texture3d,
	texturecube_array,
	texture1d_array,
	texture2d_array,

	count,

};}

namespace resource_usage
{ oDECLARE_SMALL_ENUM(value, uchar) {

	infrequent_updates,
	target,
	readback,
	unordered,

	count,

};}

struct resource_info
{
	resource_info()
		: type(resource_type::unknown)
		, usage(resource_usage::infrequent_updates)
		, format(surface::unknown)
		, mips(false)
		, dimensions(0, 0, 0)
		, array_size(0)
	{}

	resource_type::value type;
	resource_usage::value usage;
	surface::format format;
	bool mips;
	ushort3 dimensions; // for a buffer x is struct size
	ushort array_size;
};

class device;
class resource {};
class device_readable : resource {};
class device_writable : resource {};
class buffer : device_readable {};
class texture : device_readable {};
class target : device_writable {};
class unordered : device_writable {};
class readback : resource {};

resource_info get_info(resource* r);

resource* make_resource(device* dev, const resource_info& info, const char* debug_name = "");
void unmake_resource(resource* r);

inline buffer* make_buffer(device* dev, const resource_info& info, const char* debug_name = "") { return (buffer*)make_resource(dev, info, debug_name); }
inline texture* make_texture(device* dev, const resource_info& info, const char* debug_name = "") { return (texture*)make_resource(dev, info, debug_name); }
inline target* make_target(device* dev, const resource_info& info, const char* debug_name = "") { return (target*)make_resource(dev, info, debug_name); }
inline unordered* make_unordered(device* dev, const resource_info& info, const char* debug_name = "") { return (unordered*)make_resource(dev, info, debug_name); }
inline readback* make_readback(device* dev, const resource_info& info, const char* debug_name = "") { return (readback*)make_resource(dev, info, debug_name); }

readback* make_readback_copy(resource* r);

target* make_target(target* sibling, uint array_slice, const char* debug_name = "");

template<typename resourceT> struct delete_resource { void operator()(resourceT* r) const { unmake_resource((resource*)r); } };
typedef std::unique_ptr<resource, delete_resource<resource>> unique_resource_ptr;
typedef std::unique_ptr<device_readable, delete_resource<device_readable>> unique_device_readable_ptr;
typedef std::unique_ptr<device_writable, delete_resource<device_writable>> unique_device_writable_ptr;
typedef std::unique_ptr<buffer, delete_resource<buffer>> unique_buffer_ptr;
typedef std::unique_ptr<texture, delete_resource<texture>> unique_texture_ptr;
typedef std::unique_ptr<target, delete_resource<target>> unique_target_ptr;
typedef std::unique_ptr<unordered, delete_resource<unordered>> unique_unordered_ptr;
typedef std::unique_ptr<readback, delete_resource<readback>> unique_readback_ptr;

	} // namespace gpu
} // namespace ouro

#endif
