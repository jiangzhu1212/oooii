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
#include <oSurface/resize.h>
#include <oBase/fixed_vector.h>
#include <oBase/memory.h>
#include <vector>
#include <float.h>

namespace ouro {

const char* as_string(const surface::filter& _Filter)
{
	switch (_Filter)
	{
		case surface::filter::point: return "point";
		case surface::filter::box: return "box";
		case surface::filter::triangle: return "triangle";
		case surface::filter::lanczos2: return "lanczos2";
		case surface::filter::lanczos3: return "lanczos3";
		default: break;
	}
	return "?";
}

	namespace surface {

template<typename T> T sinc(T _Value)
{
	if (abs(_Value) > std::numeric_limits<T>::epsilon())
	{
		static const double PI = 3.1415926535897932384626433832795f;
		_Value *= T(PI);
		return sin(_Value) / _Value;
	} 
	return T(1);
}

struct filter_point
{
	static const int Width = 0;
protected:
	float value(float _Offset) const
	{
		if (abs(_Offset) < std::numeric_limits<float>::epsilon())
			return 1.0f;
		else
			return 0.0f;
	}
};

struct filter_box
{
	static const int Width = 1;
protected:
	float value(float _Offset) const
	{
		if (abs(_Offset) <= 1.0f)
			return 1.0f;
		else
			return 0.0f;
	}
};

struct filter_triangle
{
	static const int Width = 1;
protected:
	float value(float _Offset) const
	{
		float absOffset = abs(_Offset);
		if (absOffset <= 1.0f)
			return 1.0f - absOffset;
		else
			return 0.0f;
	}
};

struct filter_lanczos2
{
	static const int Width = 2;
protected:
	float value(float _Offset) const
	{
		const float invWidth = 1.0f/Width;
		float absOffset = abs(_Offset);
		if (absOffset <= Width)
			return sinc(_Offset) * sinc(_Offset*invWidth);
		else
			return 0.0f;
	}
};

struct filter_lanczos3
{
	static const int Width = 3;
protected:
	float value(float _Offset) const
	{
		const float invWidth = 1.0f/Width;
		float absOffset = abs(_Offset);
		if (absOffset <= Width)
			return sinc(_Offset) * sinc(_Offset*invWidth);
		else
			return 0.0f;
	}
};

template<typename T>
struct Filter : public T
{
	static const int Support = 2*Width + 1;
	struct Entry
	{
		fixed_vector<float, Support> Cache;
		int Left, Right;
	};
	std::vector<Entry> FilterCache;

	void InitFilter(int srcDim, int dstDim)
	{
		FilterCache.resize(dstDim);

		//this is a hack for magnification. need to widen the filter since our source is actually discrete, but the math is mostly continuous, otherwise nothing to grab from adjacent pixels.
		float scale = dstDim/(float)srcDim;
		if (scale <= 1.0f)
			scale = 1.0f;
		scale = 1.0f/scale;

		float halfPixel = 0.5f/dstDim;
		float srcHalfPixel = 0.5f/srcDim;

		for (int i = 0; i < dstDim; i++)
		{
			float dstCenter = i / (float)dstDim + halfPixel;
			int closestSource = static_cast<int>(round((dstCenter - srcHalfPixel) * srcDim));
			
			auto& entry = FilterCache[i];
			entry.Left = std::max(closestSource - Width, 0);
			entry.Right = std::min(closestSource + Width, srcDim-1);

			float totalWeight = 0;
			for (int j = entry.Left; j <= entry.Right; ++j)
			{
				float loc = (j / (float)srcDim) + srcHalfPixel;
				float filterLoc = (loc - dstCenter)*dstDim*scale;
				float weight = value(filterLoc);
				if (abs(weight) < std::numeric_limits<float>::epsilon() && entry.Cache.empty()) //strip any if we don't have a real weight yet.
				{
					entry.Left++;
				}
				else
				{
					totalWeight += weight;
					entry.Cache.push_back(weight);
				}
			}
			for (int j = entry.Right; j >= entry.Left; --j)
			{
				if (abs(entry.Cache[j - entry.Left]) < std::numeric_limits<float>::epsilon())
				{
					--entry.Right;
					entry.Cache.pop_back();
				}
				else
				{
					break;
				}
			}

			for (int j = entry.Left; j <= entry.Right; ++j)
			{
				entry.Cache[j - entry.Left] /= totalWeight;
			}
		}
	}
};

template<typename FILTER, size_t ELEMENT_SIZE>
void resize_horizontal(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst)
{
	if (src_info.dimensions.y != dst_info.dimensions.y)
		throw std::invalid_argument("Horizontal resize assumes y dimensions are the same for source and destination");

	FILTER filter;
	filter.InitFilter(src_info.dimensions.x, dst_info.dimensions.x);

	const char* srcData = (char*)src.data;
	char* dstData = (char*)dst.data;

	for (int y = 0; y < dst_info.dimensions.y; y++)
	{
		const uchar* oRESTRICT srcRow = (uchar*)byte_add(srcData, y*src.row_pitch);
		uchar* oRESTRICT dstRow = (uchar*)byte_add(dstData, y*dst.row_pitch);

		for (int x = 0; x < dst_info.dimensions.x; x++)
		{
			auto& filterEntry = filter.FilterCache[x];
			std::array<float, ELEMENT_SIZE> result;
			result.fill(0.0f);

			for (int srcX = filterEntry.Left; srcX <= filterEntry.Right; srcX++)
			{
				for (size_t i = 0; i < ELEMENT_SIZE; i++)
				{
					float src = srcRow[srcX*ELEMENT_SIZE + i];
					src *= filterEntry.Cache[srcX - filterEntry.Left];
					result[i] += src;
				}
			}
			
			for (size_t i = 0; i < ELEMENT_SIZE; i++)
				dstRow[x*ELEMENT_SIZE + i] = static_cast<uchar>(clamp(result[i], 0.0f, 255.0f));
		}
	}
}

template<typename FILTER, int ELEMENT_SIZE>
void resize_vertical(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst)
{
	if (src_info.dimensions.x != dst_info.dimensions.x)
		throw std::invalid_argument("vertical resize assumes x dimensions are the same for source and destination");

	FILTER filter;
	filter.InitFilter(src_info.dimensions.y, dst_info.dimensions.y);

	const uchar* oRESTRICT srcData = (uchar*)src.data;
	uchar* oRESTRICT dstData = (uchar*)dst.data;

	for(int y = 0; y < dst_info.dimensions.y; y++)
	{
		uchar* oRESTRICT dstRow = byte_add(dstData, y*dst.row_pitch);

		for(int x = 0; x < dst_info.dimensions.x; ++x)
		{
			auto& filterEntry = filter.FilterCache[y];
			std::array<float, ELEMENT_SIZE> result;
			result.fill(0.0f);

			for (int srcY = filterEntry.Left;srcY <= filterEntry.Right; ++srcY)
			{
				const uchar* oRESTRICT srcElement = byte_add(srcData, srcY*src.row_pitch + x*ELEMENT_SIZE);

				for (size_t i = 0;i < ELEMENT_SIZE; i++)
				{
					float src = srcElement[i];
					src *= filterEntry.Cache[srcY - filterEntry.Left];
					result[i] += src;
				}
			}
			for (size_t i = 0;i < ELEMENT_SIZE; i++)
			{
				dstRow[x*ELEMENT_SIZE + i] = static_cast<uchar>(clamp(result[i], 0.0f, 255.0f));
			}
		}
	}
}

template<typename FILTER, int ELEMENT_SIZE>
void resize_internal(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst)
{
	// Assuming all our filters are separable for now.

	if (all(src_info.dimensions == dst_info.dimensions)) // no actual resize, just copy
		copy(src_info, src, dst);
	else if (FILTER::Support == 1) // point sampling
	{		
		const char* srcData = (char*)src.data;
		char* dstData = (char*)dst.data;

		// Bresenham style for x
		int fixedStep = (src_info.dimensions.x / dst_info.dimensions.x)*ELEMENT_SIZE;
		int remainder = (src_info.dimensions.x % dst_info.dimensions.x);

		for (int y = 0; y < dst_info.dimensions.y; y++)
		{
			int row = (y*src_info.dimensions.y)/dst_info.dimensions.y;
			const char* oRESTRICT srcRow = byte_add(srcData, row*src.row_pitch);
			char* oRESTRICT dstRow = byte_add(dstData, y*dst.row_pitch);

			int step = 0;
			for(int x = 0; x < dst_info.dimensions.x; ++x)
			{
				for (size_t i = 0; i < ELEMENT_SIZE; i++)
					*dstRow++ = *(srcRow+i);
				srcRow += fixedStep;
				step += remainder;
				if (step >= dst_info.dimensions.x)
				{
					srcRow += ELEMENT_SIZE;
					step -= dst_info.dimensions.x;
				}
			}
		}
	}
	else // have to run a real filter
	{
		if (src_info.dimensions.x == dst_info.dimensions.x) //Only need a y filter
			resize_vertical<FILTER, ELEMENT_SIZE>(src_info, src, dst_info, dst);
		else if (src_info.dimensions.y == dst_info.dimensions.y) //Only need a x filter
			resize_horizontal<FILTER, ELEMENT_SIZE>(src_info, src, dst_info, dst);
		else if (dst_info.dimensions.x*src_info.dimensions.y <= dst_info.dimensions.y*src_info.dimensions.x) // more efficient to run x filter then y
		{
			info tempInfo = src_info;
			tempInfo.dimensions.x = dst_info.dimensions.x;
			tempInfo.layout = surface::layout::tight;
			std::vector<char> tempImage;
			tempImage.resize(total_size(tempInfo));

			mapped_subresource tempMap = get_mapped_subresource(tempInfo, 0, 0, tempImage.data());
			resize_horizontal<FILTER, ELEMENT_SIZE>(src_info, src, tempInfo, tempMap);
			const_mapped_subresource tempMapConst = tempMap;
			resize_vertical<FILTER, ELEMENT_SIZE>(tempInfo, tempMapConst, dst_info, dst);
		}
		else // more efficient to run y filter then x
		{
			info tempInfo = src_info;
			tempInfo.dimensions.y = dst_info.dimensions.y;
			tempInfo.layout = surface::layout::tight;
			std::vector<char> tempImage; // todo: fix internal allocation
			tempImage.resize(total_size(tempInfo));

			mapped_subresource tempMap = get_mapped_subresource(tempInfo, 0, 0, tempImage.data());
			resize_vertical<FILTER, ELEMENT_SIZE>(src_info, src, tempInfo, tempMap);
			const_mapped_subresource tempMapConst = tempMap;
			resize_horizontal<FILTER, ELEMENT_SIZE>(tempInfo, tempMapConst, dst_info, dst);
		}
	}
}

void resize(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst, const filter& f)
{
	if (src_info.layout != dst_info.layout || src_info.format != dst_info.format)
		throw std::invalid_argument("incompatible surfaces");

	if (is_block_compressed(src_info.format))
		throw std::invalid_argument("block compressed formats cannot be resized");

	const int elementSize = element_size(src_info.format);

	#define FILTER_CASE(filter_type) \
	case filter::filter_type: \
	{	switch (elementSize) \
		{	case 1: resize_internal<Filter<filter_##filter_type>,1>(src_info, src, dst_info, dst); break; \
			case 2: resize_internal<Filter<filter_##filter_type>,2>(src_info, src, dst_info, dst); break; \
			case 3: resize_internal<Filter<filter_##filter_type>,3>(src_info, src, dst_info, dst); break; \
			case 4: resize_internal<Filter<filter_##filter_type>,4>(src_info, src, dst_info, dst); break; \
			oNODEFAULT; \
		} \
		break; \
	}

	switch (f)
	{
		FILTER_CASE(point)
		FILTER_CASE(box)
		FILTER_CASE(triangle)
		FILTER_CASE(lanczos2)
		FILTER_CASE(lanczos3)
		default: throw std::invalid_argument("unsupported filter type");
	}

	#undef FILTER_CASE
}

void clip(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst, int2 src_offset)
{
	if (src_info.layout != dst_info.layout || src_info.format != dst_info.format)
		throw std::invalid_argument("incompatible surfaces");

	if (ouro::surface::is_block_compressed(src_info.format))
		throw std::invalid_argument("block compressed formats cannot be clipped");

	if (src_offset.x < 0 || src_offset.y < 0)
		throw std::invalid_argument("source offset must be >= 0");

	int2 bottomRight = src_offset + dst_info.dimensions.xy();
	if (bottomRight.x > src_info.dimensions.x || bottomRight.y > src_info.dimensions.y)
		throw std::invalid_argument("src_offset + the dimensions of the destination, must be within the dimensions of the source");

	int elementSize = ouro::surface::element_size(src_info.format);
	memcpy2d(dst.data, dst.row_pitch, byte_add(src.data, src.row_pitch*src_offset.y + elementSize*src_offset.x), src.row_pitch, dst_info.dimensions.x*elementSize, dst_info.dimensions.y);
}

void pad(const info& src_info, const const_mapped_subresource& src, const info& dst_info, const mapped_subresource& dst, int2 dst_offset)
{
	if (src_info.layout != dst_info.layout || src_info.format != dst_info.format)
		throw std::invalid_argument("incompatible surfaces");

	if (is_block_compressed(src_info.format))
		throw std::invalid_argument("block compressed formats cannot be padded");

	if (any(dst_offset < int2(0,0)))
		throw std::invalid_argument("destination offset must be >= 0");

	int2 bottomRight = dst_offset + src_info.dimensions.xy();
	if (any(bottomRight > dst_info.dimensions.xy()))
		throw std::invalid_argument("dst_offset + destination dimensions must be within the dimensions of the source");

	int elementSize = element_size(src_info.format);
	memcpy2d(byte_add(dst.data, dst.row_pitch*dst_offset.y + elementSize*dst_offset.x), dst.row_pitch, src.data, src.row_pitch, src_info.dimensions.x*elementSize, src_info.dimensions.y);
}

}}
