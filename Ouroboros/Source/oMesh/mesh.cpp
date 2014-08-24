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
#include <oMesh/mesh.h>
#include <oBase/throw.h>
#include "mesh_template.h"

#define STR_SUPPORT(_T, _NumTs) \
	oDEFINE_TO_STRING(_T) \
	oDEFINE_FROM_STRING(_T, _NumTs)

namespace ouro {

const char* as_string(const mesh::primitive_type::value& _Value)
{
	switch (_Value)
	{
		case mesh::primitive_type::unknown: return "unknown";
		case mesh::primitive_type::points: return "points";
		case mesh::primitive_type::lines: return "lines";
		case mesh::primitive_type::line_strips: return "line_strips";
		case mesh::primitive_type::triangles: return "triangles";
		case mesh::primitive_type::triangle_strips: return "triangle_strips";
		case mesh::primitive_type::lines_adjacency: return "lines_adjacency";
		case mesh::primitive_type::line_strips_adjacency: return "line_strips_adjacency";
		case mesh::primitive_type::triangles_adjacency: return "triangles_adjacency";
		case mesh::primitive_type::triangle_strips_adjacency: return "triangle_strips_adjacency";
		case mesh::primitive_type::patches1: return "patches1";
		case mesh::primitive_type::patches2: return "patches2";
		case mesh::primitive_type::patches3: return "patches3";
		case mesh::primitive_type::patches4: return "patches4";
		case mesh::primitive_type::patches5: return "patches5";
		case mesh::primitive_type::patches6: return "patches6";
		case mesh::primitive_type::patches7: return "patches7";
		case mesh::primitive_type::patches8: return "patches8";
		case mesh::primitive_type::patches9: return "patches9";
		case mesh::primitive_type::patches10: return "patches10";
		case mesh::primitive_type::patches11: return "patches11";
		case mesh::primitive_type::patches12: return "patches12";
		case mesh::primitive_type::patches13: return "patches13";
		case mesh::primitive_type::patches14: return "patches14";
		case mesh::primitive_type::patches15: return "patches15";
		case mesh::primitive_type::patches16: return "patches16";
		case mesh::primitive_type::patches17: return "patches17";
		case mesh::primitive_type::patches18: return "patches18";
		case mesh::primitive_type::patches19: return "patches19";
		case mesh::primitive_type::patches20: return "patches20";
		case mesh::primitive_type::patches21: return "patches21";
		case mesh::primitive_type::patches22: return "patches22";
		case mesh::primitive_type::patches23: return "patches23";
		case mesh::primitive_type::patches24: return "patches24";
		case mesh::primitive_type::patches25: return "patches25";
		case mesh::primitive_type::patches26: return "patches26";
		case mesh::primitive_type::patches27: return "patches27";
		case mesh::primitive_type::patches28: return "patches28";
		case mesh::primitive_type::patches29: return "patches29";
		case mesh::primitive_type::patches30: return "patches30";
		case mesh::primitive_type::patches31: return "patches31";
		case mesh::primitive_type::patches32: return "patches32";

		default: break;
	}
	return "?";
}

STR_SUPPORT(mesh::primitive_type::value, mesh::primitive_type::count);

const char* as_string(const mesh::face_type::value& _Value)
{
	switch (_Value)
	{
		case mesh::face_type::unknown: return "unknown";
		case mesh::face_type::front_ccw: return "front_ccw";
		case mesh::face_type::front_cw: return "front_cw";
		case mesh::face_type::outline: return "outline";
		default: break;
	}
	return "?";
}
		
STR_SUPPORT(mesh::face_type::value, mesh::face_type::count);

namespace mesh {

uint calc_offset(const element_array& elements, uint element_index)
{
	uint offset = 0;
	const uint slot = elements[element_index].slot();
	for (size_t i = 0; i < element_index; i++)
		if (elements[i].slot() == slot)
			offset += surface::element_size(elements[i].format());
	return offset;
}

uint calc_vertex_size(const element_array& elements, uint slot)
{
	uint size = 0;
	for (const element& e : elements)
		if (e.slot() == slot)
			size += surface::element_size(e.format());
	return size;
}

uint num_primitives(const primitive_type::value& type, uint num_indices, uint num_vertices)
{
	uint n = num_indices;
	switch (type)
	{
		case primitive_type::points: n = num_vertices; break;
		case primitive_type::lines: n /= 2; break;
		case primitive_type::line_strips: n--; break;
		case primitive_type::triangles: n /= 3; break;
		case primitive_type::triangle_strips: n -= 2; break;
		default: oTHROW_INVARG("unsupported primitive type");
	}
	return n;
}

void flip_winding_order(uint base_index_index, ushort* indices, uint num_indices)
{
	oCHECK((base_index_index % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	oCHECK((num_indices % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	for (uint i = base_index_index; i < num_indices; i += 3)
		std::swap(indices[i+1], indices[i+2]);
}

void flip_winding_order(uint base_index_index, uint* indices, uint num_indices)
{
	oCHECK((base_index_index % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	oCHECK((num_indices % 3) == 0, "Indices is not divisible by 3, so thus is not triangles and cannot be re-winded");
	for (uint i = base_index_index; i < num_indices; i += 3)
		std::swap(indices[i+1], indices[i+2]);
}

void copy_indices(void* oRESTRICT dst, uint dst_pitch, const void* oRESTRICT src, uint src_pitch, uint num_indices)
{
	if (dst_pitch == sizeof(uint) && src_pitch == sizeof(ushort))
		copy_indices((uint*)dst, (const ushort*)src, num_indices);
	else if (dst_pitch == sizeof(ushort) && src_pitch == sizeof(uint))
		copy_indices((ushort*)dst, (const uint*)src, num_indices);
	else
		oTHROW_INVARG("unsupported index pitches (src=%d, dst=%d)", src_pitch, dst_pitch);
}

void copy_indices(ushort* oRESTRICT dst, const uint* oRESTRICT src, uint num_indices)
{
	const uint* end = &src[num_indices];
	while (src < end)
	{
		if (*src > 65535) oTHROW_INVARG("truncating a uint (%d) to a ushort in a way that will change its value.", *src);
		*dst++ = (*src++) & 0xffff;
	}
}

void copy_indices(uint* oRESTRICT dst, const ushort* oRESTRICT src, uint num_indices)
{
	const ushort* end = &src[num_indices];
	while (src < end)
		*dst++ = *src++;
}

void offset_indices(ushort* oRESTRICT dst, uint num_indices, int offset)
{
	const ushort* end = dst + num_indices;
	while (dst < end)
	{
		uint i = *dst + offset;
		if ( i & 0xffff0000)
			throw std::out_of_range("indices with offset push it out of range");
		*dst++ = static_cast<ushort>(i);
	}
}

void offset_indices(uint* oRESTRICT dst, uint num_indices, int offset)
{
	const uint* end = dst + num_indices;
	while (dst < end)
	{
		ullong i = *dst + offset;
		if ( i & 0xffffffff00000000)
			throw std::out_of_range("indices with offset push it out of range");
		*dst++ = static_cast<uint>(i);
	}
}

template<typename DstT, typename SrcT>
static void copy_vertex_element(DstT* oRESTRICT dst, uint dst_pitch, const SrcT* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	if (std::is_same<DstT, SrcT>::value)
		memcpy2d(dst, dst_pitch, src, src_pitch, src_pitch, num_vertices);
	else
	{
		const DstT* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
		while (dst < end)
		{
			*dst = *src;
			dst = byte_add(dst, dst_pitch);
			src = byte_add(src, src_pitch);
		}
	}
}

template<>
static void copy_vertex_element<float3, float4>(float3* oRESTRICT dst, uint dst_pitch, const float4* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const float3* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y; 
		dst->z = src->z;
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

template<>
static void copy_vertex_element<float2, float3>(float2* oRESTRICT dst, uint dst_pitch, const float3* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const float2* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y; 
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

template<>
static void copy_vertex_element<float3, float2>(float3* oRESTRICT dst, uint dst_pitch, const float2* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const float3* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y;
		dst->z = 0.0f;
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

//template<>
//static void copy_vertex_element<float2, ushort2>(float2* oRESTRICT dst, uint dst_pitch, const ushort2* oRESTRICT src, uint src_pitch, uint num_vertices)
//{
//	const float2* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
//	while (dst < end)
//	{
//		dst->x = n16tof32(src->x);
//		dst->y = n16tof32(src->y);
//		dst = byte_add(dst, dst_pitch);
//		src = byte_add(src, src_pitch);
//	}
//}

//template<>
//static void copy_vertex_element<float2, ushort4>(float2* oRESTRICT dst, uint dst_pitch, const ushort4* oRESTRICT src, uint src_pitch, uint num_vertices)
//{
//	const float2* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
//	while (dst < end)
//	{
//		dst->x = n16tof32(src->x);
//		dst->y = n16tof32(src->y);
//		dst = byte_add(dst, dst_pitch);
//		src = byte_add(src, src_pitch);
//	}
//}

template<>
static void copy_vertex_element<half2, float3>(half2* oRESTRICT dst, uint dst_pitch, const float3* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const half2* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y;
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

//template<>
//static void copy_vertex_element<ushort4, float3>(ushort4* oRESTRICT dst, uint dst_pitch, const float3* oRESTRICT src, uint src_pitch, uint num_vertices)
//{
//	const ushort4* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
//	while (dst < end)
//	{
//		dst->x = f32ton16(src->x);
//		dst->y = f32ton16(src->y); 
//		dst->z = f32ton16(src->z);
//		dst->w = 65535;//f32ton16(1.0f);
//		dst = byte_add(dst, dst_pitch);
//		src = byte_add(src, src_pitch);
//	}
//}

//template<>
//static void copy_vertex_element<float3, ushort4>(float3* oRESTRICT dst, uint dst_pitch, const ushort4* oRESTRICT src, uint src_pitch, uint num_vertices)
//{
//	const float3* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
//	while (dst < end)
//	{
//		dst->x = n16tof32(src->x);
//		dst->y = n16tof32(src->y); 
//		dst->z = n16tof32(src->z);
//		dst = byte_add(dst, dst_pitch);
//		src = byte_add(src, src_pitch);
//	}
//}

template<>
static void copy_vertex_element<half4, float3>(half4* oRESTRICT dst, uint dst_pitch, const float3* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const half4* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y;
		dst->z = src->z;
		dst->w = 0.0f;
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

template<>
static void copy_vertex_element<float3, half4>(float3* oRESTRICT dst, uint dst_pitch, const half4* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const float3* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y;
		dst->z = src->z;
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

template<>
static void copy_vertex_element<half4, float4>(half4* oRESTRICT dst, uint dst_pitch, const float4* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const half4* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y;
		dst->z = src->z;
		dst->w = src->w;
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

template<>
static void copy_vertex_element<float4, half4>(float4* oRESTRICT dst, uint dst_pitch, const half4* oRESTRICT src, uint src_pitch, uint num_vertices)
{
	const float4* oRESTRICT end = byte_add(dst, dst_pitch * num_vertices);
	while (dst < end)
	{
		dst->x = src->x;
		dst->y = src->y;
		dst->z = src->z;
		dst->w = src->w;
		dst = byte_add(dst, dst_pitch);
		src = byte_add(src, src_pitch);
	}
}

//template<typename DstT, typename SrcT>
//static void copy_semantic(void* oRESTRICT & dst, uint dst_pitch, const layout::value& _DestinationLayout
//	, const SrcT* oRESTRICT src, uint src_pitch, uint num_vertices
//	, bool (*has_element)(const layout::value& _Layout))
//{
//	if (has_element(_DestinationLayout))
//	{
//		if (src)
//			copy_vertex_element<DstT, SrcT>((DstT*)dst, dst_pitch, (SrcT*)src, src_pitch, num_vertices);
//		else
//			memset2d4(dst, dst_pitch, 0, sizeof(DstT), num_vertices);
//
//		dst = byte_add(dst, sizeof(DstT));
//	}
//}

uint copy_element(uint dst_byte_offset, void* oRESTRICT dst, uint dst_stride, const surface::format& dst_format,
									 const void* oRESTRICT src, uint src_stride, const surface::format& src_format, uint num_vertices)
{
	const uint DstSize = surface::element_size(dst_format);
	const uint SrcSize = surface::element_size(src_format);

	void* oRESTRICT dest = byte_add(dst, dst_byte_offset);

	bool DidCopy = false;
	switch (dst_format)
	{
		#define COPY(DstFmt, SrcFmt, DstType, SrcType) case surface::format::SrcFmt: { copy_vertex_element((DstType*)dest, dst_stride, (const SrcType*)src, src_stride, num_vertices); DidCopy = true; break; }
		case surface::format::r32g32_float:
		{
			switch (src_format)
			{
				COPY(r32g32_float, r32g32_float, float2, float2)
				COPY(r32g32_float, r32g32b32_float, float2, float3)
				COPY(r32g32_float, r16g16_unorm, float2, ushort2)
				//COPY(r32g32_float, r16g16b16a16_unorm, float2, ushort4)
				COPY(r32g32_float, r16g16b16a16_float, float3, half4)
				default: break;
			}
			break;
		}

		case surface::format::r32g32b32_float:
		{
			switch (src_format)
			{
				COPY(r32g32b32_float, r32g32b32_float, float3, float3)
				COPY(r32g32b32_float, r32g32_float, float3, float2)
				COPY(r32g32b32_float, r32g32b32a32_float, float3, float4)
				COPY(r32g32b32_float, r10g10b10a2_unorm, float3, udec3)
				//COPY(r32g32b32_float, r16g16b16a16_unorm, float3, ushort4)
				COPY(r32g32b32_float, r16g16b16a16_float, float3, half4)
				default: break;
			}
			break;
		}

		case surface::format::r32g32b32a32_float:
		{
			switch (src_format)
			{
				COPY(r32g32b32a32_float, r32g32b32a32_float, float4, float4)
				COPY(r32g32b32a32_float, r16g16b16a16_unorm, float4, ushort4)
				COPY(r32g32b32a32_float, r16g16b16a16_float, float4, half4)
				default: break;
			}
			break;
		}

		//case surface::format::r10g10b10a2_unorm:
		//{
		//	switch (src_format)
		//	{
		//		COPY(r10g10b10a2_unorm, r10g10b10a2_unorm, udec3, udec3)
		//		COPY(r10g10b10a2_unorm, r32g32b32_float, udec3, float3)
		//		default: break;
		//	}
		//	break;
		//}

		case surface::format::r10g10b10a2_uint:
		{
			switch (src_format)
			{
				COPY(r10g10b10a2_uint, r10g10b10a2_uint, udec3, udec3)
				default: break;
			}
			break;
		}

		//case surface::format::r16g16_unorm:
		//{
		//	switch (src_format)
		//	{
		//		COPY(r16g16_unorm, r16g16_unorm, ushort2, ushort2)
		//		default: break;
		//	}
		//	break;
		//}

		case surface::format::r16g16_uint:
		{
			switch (src_format)
			{
				COPY(r16g16_uint, r16g16_uint, ushort2, ushort2)
				default: break;
			}
			break;
		}

		case surface::format::r16g16_float:
		{
			switch (src_format)
			{
				COPY(r16g16_float, r16g16_float, half2, half2)
				COPY(r16g16_float, r32g32_float, half2, float2)
				COPY(r16g16_float, r32g32b32_float, half2, float3)
				default: break;
			}
			break;
		}

		//case surface::format::r16g16b16a16_unorm:
		//{
		//	switch (src_format)
		//	{
		//		COPY(r16g16b16a16_unorm, r16g16b16a16_unorm, ushort4, ushort4)
		//		COPY(r16g16b16a16_unorm, r32g32b32_float, ushort4, float3)
		//		COPY(r16g16b16a16_unorm, r32g32b32a32_float, ushort4, float4)
		//		default: break;
		//	}
		//	break;
		//}

		case surface::format::r16g16b16a16_uint:
		{
			switch (src_format)
			{
				COPY(r16g16b16a16_uint, r16g16b16a16_uint, ushort4, ushort4)
				default: break;
			}
			break;
		}

		case surface::format::r16g16b16a16_float:
		{
			switch (src_format)
			{
				COPY(r16g16b16a16_float, r16g16b16a16_float, half4, half4)
				COPY(r16g16b16a16_float, r32g32b32_float, half4, float3)
				COPY(r16g16b16a16_float, r32g32b32a32_float, half4, float4)
				default: break;
			}
			break;
		}

		case surface::format::r8g8b8a8_unorm:
		{
			switch (src_format)
			{
				COPY(r8g8b8a8_unorm, r8g8b8a8_unorm, uint, uint)
				default: break;
			}
			break;
		}

		case surface::format::r8g8b8a8_uint:
		{
			switch (src_format)
			{
				COPY(r8g8b8a8_uint, r8g8b8a8_uint, uint, uint)
				default: break;
			}
			break;
		}

		case surface::format::b8g8r8a8_unorm:
		{
			switch (src_format)
			{
				COPY(b8g8r8a8_unorm, b8g8r8a8_unorm, color, color)
				default: break;
			}
			break;
		}

		case surface::format::b8g8r8a8_unorm_srgb:
		{
			switch (src_format)
			{
				COPY(b8g8r8a8_unorm_srgb, b8g8r8a8_unorm_srgb, color, color)
				default: break;
			}
			break;
		}

		default: 
			break;
	}
	#undef COPY

	if (!DidCopy)
		memset2d4(dst, dst_stride, 0, DstSize, num_vertices);

	return dst_byte_offset + DstSize;
}

void copy_vertices(void* oRESTRICT* oRESTRICT dst, const element_array& dst_elements, const void* oRESTRICT* oRESTRICT src, const element_array& src_elements, uint num_vertices)
{
	for (uint di = 0; di < as_uint(dst_elements.size()); di++)
	{
		const element& e = dst_elements[di];
		if (e.semantic() == surface::semantic::unknown)
			continue;

		const uint dslot = e.slot();

		if (!dst[dslot])
			continue;

		const uint doff = calc_offset(dst_elements, di);
		const uint dstride = calc_vertex_size(dst_elements, dslot);

		bool copied = false;
		for (uint si = 0; si < src_elements.size(); si++)
		{
			const element& se = src_elements[si];
			const uint sslot = se.slot();
			if (e.semantic() == se.semantic() && e.index() == se.index() && src[sslot])
			{
				const uint soff = calc_offset(src_elements, si);
				const uint sstride = calc_vertex_size(src_elements, sslot);

				copy_element(doff, dst[dslot], dstride, e.format(), src[sslot], sstride, se.format(), num_vertices);
				copied = true;
				break;
			}
		}

		if (!copied)
			memset2d4(dst[e.slot()], dstride, 0, surface::element_size(e.format()), num_vertices);
	}
}

void calc_min_max_indices(const uint* oRESTRICT indices, uint start_index, uint num_indices, uint num_vertices, uint* oRESTRICT out_min_vertex, uint* oRESTRICT out_max_vertex)
{
	detail::calc_min_max_indices(indices, start_index, num_indices, num_vertices, out_min_vertex, out_max_vertex);
}

void calc_min_max_indices(const ushort* oRESTRICT indices, uint start_index, uint num_indices, uint num_vertices, uint* oRESTRICT out_min_vertex, uint* oRESTRICT out_max_vertex)
{
	detail::calc_min_max_indices(indices, start_index, num_indices, num_vertices, out_min_vertex, out_max_vertex);
}

aaboxf calc_bound(const float3* vertices, uint vertex_stride, uint num_vertices)
{
	return detail::calc_bound(vertices, vertex_stride, num_vertices);
}

void transform_points(const float4x4& matrix, float3* oRESTRICT dst, uint dst_stride, const float3* oRESTRICT src, uint source_stride, uint num_points)
{
	detail::transform_points(matrix, dst, dst_stride, src, source_stride, num_points);
}

void transform_vectors(const float4x4& matrix, float3* oRESTRICT dst, uint dst_stride, const float3* oRESTRICT src, uint source_stride, uint _NumVectors)
{
	detail::transform_vectors(matrix, dst, dst_stride, src, source_stride, _NumVectors);
}

void remove_degenerates(const float3* oRESTRICT positions, uint num_positions, uint* oRESTRICT indices, uint num_indices, uint* oRESTRICT out_new_num_indices)
{
	detail::remove_degenerates(positions, num_positions, indices, num_indices, out_new_num_indices);
}

void remove_degenerates(const float3* oRESTRICT positions, uint num_positions, ushort* oRESTRICT indices, uint num_indices, uint* oRESTRICT out_new_num_indices)
{
	detail::remove_degenerates(positions, num_positions, indices, num_indices, out_new_num_indices);
}

void calc_face_normals(float3* oRESTRICT face_normals, const uint* oRESTRICT indices, uint num_indices, const float3* oRESTRICT positions, uint num_positions, bool ccw)
{
	detail::calc_face_normals(face_normals, indices, num_indices, positions, num_positions, ccw);
}

void calc_face_normals(float3* oRESTRICT face_normals, const ushort* oRESTRICT indices, uint num_indices, const float3* oRESTRICT positions, uint num_positions, bool ccw)
{
	detail::calc_face_normals(face_normals, indices, num_indices, positions, num_positions, ccw);
}

void calc_vertex_normals(float3* vertex_normals, const uint* indices, uint num_indices, const float3* positions, uint num_positions, bool ccw, bool overwrite_all)
{
	detail::calc_vertex_normals(vertex_normals, indices, num_indices, positions, num_positions, ccw, overwrite_all);
}

void calc_vertex_normals(float3* vertex_normals, const ushort* indices, uint num_indices, const float3* positions, uint num_positions, bool ccw, bool overwrite_all)
{
	detail::calc_vertex_normals(vertex_normals, indices, num_indices, positions, num_positions, ccw, overwrite_all);
}

void calc_vertex_tangents(float4* tangents, const uint* indices, uint num_indices, const float3* positions, const float3* normals, const float3* texcoords, uint num_vertices)
{
	detail::calc_vertex_tangents(tangents, indices, num_indices, positions, normals, texcoords, num_vertices);
}

void calc_vertex_tangents(float4* tangents, const uint* indices, uint num_indices, const float3* positions, const float3* normals, const float2* texcoords, uint num_vertices)
{
	detail::calc_vertex_tangents(tangents, indices, num_indices, positions, normals, texcoords, num_vertices);
}

void calc_vertex_tangents(float4* tangents, const ushort* indices, uint num_indices, const float3* positions, const float3* normals, const float3* texcoords, uint num_vertices)
{
	detail::calc_vertex_tangents(tangents, indices, num_indices, positions, normals, texcoords, num_vertices);
}

void calc_vertex_tangents(float4* tangents, const ushort* indices, uint num_indices, const float3* positions, const float3* normals, const float2* texcoords, uint num_vertices)
{
	detail::calc_vertex_tangents(tangents, indices, num_indices, positions, normals, texcoords, num_vertices);
}

void calc_texcoords(const aaboxf& bound, const uint* indices, uint num_indices, const float3* positions, float2* out_texcoords, uint num_vertices, double* out_solve_time)
{
	detail::calc_texcoords(bound, indices, num_indices, positions, out_texcoords, num_vertices, out_solve_time);
}

void calc_texcoords(const aaboxf& bound, const uint* indices, uint num_indices, const float3* positions, float3* out_texcoords, uint num_vertices, double* out_solve_time)
{
	detail::calc_texcoords(bound, indices, num_indices, positions, out_texcoords, num_vertices, out_solve_time);
}

void calc_texcoords(const aaboxf& bound, const ushort* indices, uint num_indices, const float3* positions, float2* out_texcoords, uint num_vertices, double* out_solve_time)
{
	detail::calc_texcoords(bound, indices, num_indices, positions, out_texcoords, num_vertices, out_solve_time);
}

void calc_texcoords(const aaboxf& bound, const ushort* indices, uint num_indices, const float3* positions, float3* out_texcoords, uint num_vertices, double* out_solve_time)
{
	detail::calc_texcoords(bound, indices, num_indices, positions, out_texcoords, num_vertices, out_solve_time);
}

namespace TerathonEdges {

	/** <citation
	usage="Implementation" 
	reason="tangents can be derived, and this is how to do it" 
	author="Eric Lengyel"
	description="http://www.terathon.com/code/edges.html"
	license="*** Assumed Public Domain ***"
	licenseurl="http://www.terathon.com/code/edges.html"
	modification="Minor changes to not limit algo to 65536 indices and some fixes to get it compiling"
	/>*/

	// $(CitedCodeBegin)

	// Building an Edge List for an Arbitrary Mesh
	// The following code builds a list of edges for an arbitrary triangle 
	// mesh and has O(n) running time in the number of triangles n in the 
	// pGeometry-> The edgeArray parameter must point to a previously allocated 
	// array of Edge structures large enough to hold all of the mesh's 
	// edges, which in the worst possible case is 3 times the number of 
	// triangles in the pGeometry->

	// An edge list is useful for many geometric algorithms in computer 
	// graphics. In particular, an edge list is necessary for stencil 
	// shadows.

	struct Edge
	{
		unsigned int      vertexIndex[2]; 
		unsigned int      faceIndex[2];
	};


	struct Triangle
	{
		unsigned int      index[3];
	};


	long BuildEdges(long vertexCount, long triangleCount,
		const Triangle *triangleArray, Edge *edgeArray)
	{
		long maxEdgeCount = triangleCount * 3;
		unsigned int *firstEdge = new unsigned int[vertexCount + maxEdgeCount];
		unsigned int *nextEdge = firstEdge + vertexCount;

		for (long a = 0; a < vertexCount; a++) firstEdge[a] = 0xFFFFFFFF;

		// First pass over all triangles. This finds all the edges satisfying the
		// condition that the first vertex index is less than the second vertex index
		// when the direction from the first vertex to the second vertex represents
		// a counterclockwise winding around the triangle to which the edge belongs.
		// For each edge found, the edge index is stored in a linked list of edges
		// belonging to the lower-numbered vertex index i. This allows us to quickly
		// find an edge in the second pass whose higher-numbered vertex index is i.

		long edgeCount = 0;
		const Triangle *triangle = triangleArray;
		for (long a = 0; a < triangleCount; a++)
		{
			long i1 = triangle->index[2];
			for (long b = 0; b < 3; b++)
			{
				long i2 = triangle->index[b];
				if (i1 < i2)
				{
					Edge *edge = &edgeArray[edgeCount];

					edge->vertexIndex[0] = (unsigned int) i1;
					edge->vertexIndex[1] = (unsigned int) i2;
					edge->faceIndex[0] = (unsigned int) a;
					edge->faceIndex[1] = (unsigned int) a;

					long edgeIndex = firstEdge[i1];
					if (edgeIndex == 0xFFFFFFFF)
					{
						firstEdge[i1] = edgeCount;
					}
					else
					{
						for (;;)
						{
							long index = nextEdge[edgeIndex];
							if (index == 0xFFFFFFFF)
							{
								nextEdge[edgeIndex] = edgeCount;
								break;
							}

							edgeIndex = index;
						}
					}

					nextEdge[edgeCount] = 0xFFFFFFFF;
					edgeCount++;
				}

				i1 = i2;
			}

			triangle++;
		}

		// Second pass over all triangles. This finds all the edges satisfying the
		// condition that the first vertex index is greater than the second vertex index
		// when the direction from the first vertex to the second vertex represents
		// a counterclockwise winding around the triangle to which the edge belongs.
		// For each of these edges, the same edge should have already been found in
		// the first pass for a different triangle. So we search the list of edges
		// for the higher-numbered vertex index for the matching edge and fill in the
		// second triangle index. The maximum number of comparisons in this search for
		// any vertex is the number of edges having that vertex as an endpoint.

		triangle = triangleArray;
		for (long a = 0; a < triangleCount; a++)
		{
			long i1 = triangle->index[2];
			for (long b = 0; b < 3; b++)
			{
				long i2 = triangle->index[b];
				if (i1 > i2)
				{
					for (long edgeIndex = firstEdge[i2]; edgeIndex != 0xFFFFFFFF;
						edgeIndex = nextEdge[edgeIndex])
					{
						Edge *edge = &edgeArray[edgeIndex];
						if ((edge->vertexIndex[1] == (unsigned int)i1) &&
							(edge->faceIndex[0] == edge->faceIndex[1]))
						{
							edge->faceIndex[1] = (unsigned int) a;
							break;
						}
					}
				}

				i1 = i2;
			}

			triangle++;
		}

		delete[] firstEdge;
		return (edgeCount);
	}

	// $(CitedCodeEnd)

} // namespace TerathonEdges

void calc_edges(uint num_vertices, const uint* indices, uint num_indices, uint** _ppEdges, uint* out_num_edges)
{
	const uint numTriangles = num_indices / 3;
	if ((uint)((long)num_vertices) != num_vertices)
		throw std::out_of_range("num_vertices is out of range");
	
	if ((uint)((long)numTriangles) != numTriangles)
		throw std::out_of_range("numTriangles is out of range");

	TerathonEdges::Edge* edgeArray = new TerathonEdges::Edge[3 * numTriangles];

	uint numEdges = static_cast<uint>(TerathonEdges::BuildEdges(static_cast<long>(num_vertices), static_cast<long>(numTriangles), (const TerathonEdges::Triangle *)indices, edgeArray));

	// @tony: Should the allocator be exposed?
	*_ppEdges = new uint[numEdges * 2];

	for (size_t i = 0; i < numEdges; i++)
	{
		(*_ppEdges)[i*2] = edgeArray[i].vertexIndex[0];
		(*_ppEdges)[i*2+1] = edgeArray[i].vertexIndex[1];
	}

	*out_num_edges = numEdges;

	delete [] edgeArray;
}

void free_edge_list(uint* edges)
{
	delete [] edges;
}

#define PUV(IndexT, UV0T, UV1T) \
void prune_unindexed_vertices(const IndexT* oRESTRICT indices, uint num_indices \
	, float3* oRESTRICT positions, float3* oRESTRICT normals, float4* oRESTRICT tangents, UV0T* oRESTRICT texcoords0, UV1T* oRESTRICT texcoords1, color* oRESTRICT colors \
	, uint num_vertices, uint* oRESTRICT out_new_num_vertices) \
{ detail::prune_unindexed_vertices(indices, num_indices, positions, normals, tangents, texcoords0, texcoords1, colors, num_vertices, out_new_num_vertices); }

PUV(uint, float2, float2)
PUV(uint, float2, float3)
PUV(uint, float3, float2)
PUV(uint, float3, float3)
PUV(ushort, float2, float2)
PUV(ushort, float2, float3)
PUV(ushort, float3, float2)
PUV(ushort, float3, float3)

uint clip_convex(const planef& plane, const float3* oRESTRICT polygon, uint num_vertices, float3* oRESTRICT out_clipped_vertices)
{
	float3 v0;
	float d0;
	bool i0;
	
	float3 v1 = polygon[num_vertices-1];
	float d1 = sdistance(plane, v1);
	bool i1 = d1 > 0.0f;

	bool clipped = false;

	uint num_clipped_vertices = 0;
	for (uint i = 0; i < num_vertices; i++)
	{
		v0 = v1;
		d0 = d1;
		i0 = i1;

		v1 = polygon[i];
		d1 = sdistance(plane, v1);
		i1 = d1 > 0.0f;

		if (i0)
			out_clipped_vertices[num_clipped_vertices++] = v0;

		if (i0 != i1)
			out_clipped_vertices[num_clipped_vertices++] = lerp(v0, v1, d0 / (d0-d1));
	}

	return num_clipped_vertices;
}

	} // namespace gpu
} // namespace ouro
