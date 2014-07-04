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
#include <oGPU/oGPUUtil.h>
#include <oBase/finally.h>

namespace ouro {
	namespace gpu {

void commit_buffer(command_list* _pCommandList, buffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oCHECK(byte_aligned(_SizeofStruct, 16), "structs must be aligned to 16 bytes (%u bytes specified %.02f of alignment)", _SizeofStruct, _SizeofStruct / 16.0f);
	surface::mapped_subresource msr;
	msr.data = const_cast<void*>(_pStruct);
	msr.row_pitch = _SizeofStruct;
	msr.depth_pitch = _NumStructs * _SizeofStruct;
	_pCommandList->commit(_pBuffer, 0, msr);
}

void commit_buffer(device* _pDevice, buffer* _pBuffer, const void* _pStruct, uint _SizeofStruct, uint _NumStructs)
{
	oCHECK(byte_aligned(_SizeofStruct, 16), "structs must be aligned to 16 bytes (%u bytes specified %.02f of alignment)", _SizeofStruct, _SizeofStruct / 16.0f);
	surface::mapped_subresource msr;
	msr.data = const_cast<void*>(_pStruct);
	msr.row_pitch = _SizeofStruct;
	msr.depth_pitch = _NumStructs * _SizeofStruct;
	_pDevice->immediate()->commit(_pBuffer, 0, msr);
}

#if 0
std::shared_ptr<buffer> make_readback_copy(buffer* _pSource)
{
	std::shared_ptr<device> Device = _pSource->get_device();
	buffer_info i = _pSource->get_info();
	i.type = buffer_type::readback;
	sstring Name;
	snprintf(Name, "%s.Readback", _pSource->name());
	return Device->make_buffer(Name, i);
}

std::shared_ptr<texture> make_readback_copy(texture* _pSource)
{
	std::shared_ptr<device> Device = _pSource->get_device();
	texture_info i = _pSource->get_info();
	i.type = make_readback(i.type);
	sstring Name;
	snprintf(Name, "%s.Readback", _pSource->name());
	return Device->make_texture(Name, i);
}
#endif

uint read_back_counter(buffer* _pUnorderedBuffer, buffer* _pPreallocatedReadbackBuffer)
{
	buffer_info i = _pUnorderedBuffer->get_info();
	if (i.type != buffer_type::unordered_structured_append && i.type != buffer_type::unordered_structured_counter)
		oTHROW_INVARG("the specified buffer must be of type buffer_type::unordered_structured_append or buffer_type::unordered_structured_counter");

	std::shared_ptr<device> Device = _pUnorderedBuffer->get_device();

	std::shared_ptr<buffer> CounterBuffer;
	buffer* Counter = _pPreallocatedReadbackBuffer;
	if (!Counter)
	{
		sstring Name;
		snprintf(Name, "%s.Readback", _pUnorderedBuffer->name());

		buffer_info rb;
		rb.type = buffer_type::readback;
		rb.struct_byte_size = sizeof(uint);

		CounterBuffer = Device->make_buffer(Name, rb);
		Counter = CounterBuffer.get();
	}

	Device->immediate()->copy_counter(Counter, 0, _pUnorderedBuffer);

	surface::mapped_subresource msr;
	if (!Device->map_read(Counter, 0, &msr, true))
		return invalid; // pass through error
	uint c = *(uint*)msr.data;
	Device->unmap_read(Counter, 0);
	return c;
}

bool read(resource* _pSourceResource, int _Subresource, surface::mapped_subresource& _Destination, bool _FlipVertically)
{
	std::shared_ptr<device> Device = _pSourceResource->get_device();

	switch (_pSourceResource->type())
	{
		case resource_type::buffer:
		{
			buffer_info i = static_cast<buffer*>(_pSourceResource)->get_info();
			if (i.type != buffer_type::readback)
				oTHROW_INVARG("The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->name());
			break;
		}

		case resource_type::texture:
		{
			texture_info i = static_cast<texture*>(_pSourceResource)->get_info();
			if (!is_readback(i.type))
				oTHROW_INVARG("The specified resource %p %s must be a readback type.", _pSourceResource, _pSourceResource->name());
			break;
		}

		default:
			break;
	}

	surface::mapped_subresource source;
	if (Device->map_read(_pSourceResource, _Subresource, &source))
	{
		uint2 ByteDimensions = _pSourceResource->byte_dimensions(_Subresource);
		memcpy2d(_Destination.data, _Destination.row_pitch, source.data, source.row_pitch, ByteDimensions.x, ByteDimensions.y, _FlipVertically);
		Device->unmap_read(_pSourceResource, _Subresource);
		return true;
	}
	return false;
}

std::shared_ptr<texture> make_texture(device* _pDevice, const char* _Name, const surface::buffer* const* _ppSourceImages, uint _NumImages, texture_type::value _Type)
{
	if (!_NumImages)
		oTHROW_INVARG("Need at least one source image");

	surface::info si = _ppSourceImages[0]->get_info();
	si.array_size = _NumImages;

	std::shared_ptr<texture> Texture;
	texture_info i;
	i.format = si.format;
	i.type = _Type;
	i.dimensions = int3(si.dimensions.xy(), is_3d(_Type) ? _NumImages : 1);
	i.array_size = is_array(_Type) || is_cube(_Type) ? static_cast<ushort>(_NumImages) : 0;

	switch (get_type(_Type))
	{
		case texture_type::default_1d:
			if (si.dimensions.y != 1)
				oTHROW_INVARG("1D textures cannot have height");
			break;

		case texture_type::default_cube:
			if (((_NumImages) % 6) != 0)
				oTHROW_INVARG("Cube maps must be specified in sets of 6");
			break;

		default:
			break;
	}

	Texture = _pDevice->make_texture(_Name, i);

	const int NumMips = surface::num_mips(is_mipped(_Type), i.dimensions);
	surface::buffer::make_type MakeType = is_3d(_Type) 
		? (NumMips ? surface::buffer::mips3d : surface::buffer::image3d)
		: (NumMips ? surface::buffer::mips_array : surface::buffer::image_array);

	auto src = surface::buffer::make(_ppSourceImages, _NumImages, MakeType);
	auto src_si = src->get_info();
	const int nSubresources = surface::num_subresources(src_si);

	if (is_3d(_Type))
	{
		for (int subresource = 0; subresource < nSubresources; subresource++)
		{
			auto sri = surface::subresource(src_si, subresource);
			surface::box region;
			region.right = sri.dimensions.x;
			region.bottom = sri.dimensions.y;
			surface::shared_lock lock(src, subresource);
			for (int slice = 0; slice < sri.dimensions.z; slice++)
			{
				region.front = slice;
				region.back = slice + 1;
				_pDevice->immediate()->commit(Texture, subresource, lock.mapped, region);
			}
		}
	}

	else
	{
		for (int subresource = 0; subresource < nSubresources; subresource++)
		{
			surface::shared_lock lock(src, subresource);
			_pDevice->immediate()->commit(Texture, subresource, lock.mapped);
		}
	}

	return Texture;
}

#if 0
std::shared_ptr<surface::buffer> copy_to_surface_buffer(texture* _pSource, int _Subresource)
{
	std::shared_ptr<device> Device = _pSource->get_device();

	std::shared_ptr<texture> readbackTexture;
	texture* readback = nullptr;
	texture_info i = _pSource->get_info();
	if (is_readback(i.type))
		readback = _pSource;
	else
	{
		readbackTexture = make_readback_copy(_pSource);
		readback = readbackTexture.get();
		i = readback->get_info();
		Device->immediate()->copy(readback, _pSource);
	}

	surface::info si;
	si.dimensions = i.dimensions;
	si.format = i.format;
	si.layout = surface::tight;
	si.array_size = i.array_size;

	std::shared_ptr<surface::buffer> b;

	if (_Subresource == invalid)
	{
		b = surface::buffer::make(si);
		
		int nSubresources = surface::num_subresources(si);
		if (is_3d(i.type))
		{
			for (int subresource = 0; subresource < nSubresources; subresource++)
			{
				auto sri = surface::subresource(si, subresource);
				surface::box region;
				region.right = sri.dimensions.x;
				region.bottom = sri.dimensions.y;

				surface::mapped_subresource mapped;
				oCHECK(Device->map_read(readback, subresource, &mapped, true), "map_read failed");

				for (int slice = 0; slice < sri.dimensions.z; slice++)
				{
					region.front = slice;
					region.back = slice + 1;
					b->update_subresource(subresource, region, mapped);
				}

				Device->unmap_read(readback, subresource);
			}
		}

		else
		{
			for (int subresource = 0; subresource < nSubresources; subresource++)
			{
				auto sri = surface::subresource(si, subresource);

				surface::mapped_subresource mapped;
				oCHECK(Device->map_read(readback, subresource, &mapped, true), "map_read failed");
				b->update_subresource(subresource, mapped);
				Device->unmap_read(readback, subresource);
			}
		}
	}

	else
	{
		surface::subresource_info sri = surface::subresource(si, _Subresource);
		si.dimensions = sri.dimensions;
		si.format = sri.format;
		si.array_size = 0;
		si.layout = surface::image;
		b = surface::buffer::make(si);

		surface::mapped_subresource mapped;
		oCHECK(Device->map_read(readback, _Subresource, &mapped, true), "map_read failed");
		b->update_subresource(_Subresource, mapped);
		Device->unmap_read(readback, _Subresource);
	}

	return b;
}
#endif

	} // namespace gpu
} // namespace ouro
