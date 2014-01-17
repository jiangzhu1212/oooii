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

const char* as_string(const surface::filter::value& _Filter)
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

		for (int i = 0; i < dstDim; ++i)
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

template<typename FILTER, int ELEMENT_SIZE>
void resize_horizontal(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination)
{
	if (_SourceInfo.dimensions.y != _DestinationInfo.dimensions.y)
		throw std::invalid_argument("Horizontal resize assumes y dimensions are the same for source and destination");

	FILTER filter;
	filter.InitFilter(_SourceInfo.dimensions.x, _DestinationInfo.dimensions.x);

	const char* srcData = (char*)_Source.data;
	char* dstData = (char*)_pDestination->data;

	for (int y = 0; y < _DestinationInfo.dimensions.y; y++)
	{
		const unsigned char* oRESTRICT srcRow = (unsigned char*)byte_add(srcData, y*_Source.row_pitch);
		unsigned char* oRESTRICT dstRow = (unsigned char*)byte_add(dstData, y*_pDestination->row_pitch);

		for (int x = 0; x < _DestinationInfo.dimensions.x; x++)
		{
			auto& filterEntry = filter.FilterCache[x];
			std::array<float, ELEMENT_SIZE> result;
			result.fill(0.0f);

			for (int srcX = filterEntry.Left; srcX <= filterEntry.Right; srcX++)
			{
				for (int i = 0; i < ELEMENT_SIZE; ++i)
				{
					float src = srcRow[srcX*ELEMENT_SIZE + i];
					src *= filterEntry.Cache[srcX - filterEntry.Left];
					result[i] += src;
				}
			}
			
			for (int i = 0; i < ELEMENT_SIZE; i++)
				dstRow[x*ELEMENT_SIZE + i] = static_cast<unsigned char>(clamp(result[i], 0.0f, 255.0f));
		}
	}
}

template<typename FILTER, int ELEMENT_SIZE>
void resize_vertical(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination)
{
	if (_SourceInfo.dimensions.x != _DestinationInfo.dimensions.x)
		throw std::invalid_argument("vertical resize assumes x dimensions are the same for source and destination");

	FILTER filter;
	filter.InitFilter(_SourceInfo.dimensions.y, _DestinationInfo.dimensions.y);

	const unsigned char* oRESTRICT srcData = (unsigned char*)_Source.data;
	unsigned char* oRESTRICT dstData = (unsigned char*)_pDestination->data;

	for(int y = 0; y < _DestinationInfo.dimensions.y; y++)
	{
		unsigned char* oRESTRICT dstRow = byte_add(dstData, y*_pDestination->row_pitch);

		for(int x = 0; x < _DestinationInfo.dimensions.x; ++x)
		{
			auto& filterEntry = filter.FilterCache[y];
			float result[ELEMENT_SIZE];
			for (int i = 0;i < ELEMENT_SIZE; ++i)
			{
				result[i] = 0;
			}

			for (int srcY = filterEntry.Left;srcY <= filterEntry.Right; ++srcY)
			{
				const unsigned char* oRESTRICT srcElement = byte_add(srcData, srcY*_Source.row_pitch + x*ELEMENT_SIZE);

				for (int i = 0;i < ELEMENT_SIZE; ++i)
				{
					float src = srcElement[i];
					src *= filterEntry.Cache[srcY - filterEntry.Left];
					result[i] += src;
				}
			}
			for (int i = 0;i < ELEMENT_SIZE; ++i)
			{
				dstRow[x*ELEMENT_SIZE + i] = static_cast<unsigned char>(clamp(result[i], 0.0f, 255.0f));
			}
		}
	}
}

template<typename FILTER, int ELEMENT_SIZE>
void resize_internal(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination)
{
	// Assuming all our filters are separable for now.

	if (all(_SourceInfo.dimensions == _DestinationInfo.dimensions)) // no actual resize, just copy
		copy(_SourceInfo, _Source, _pDestination, false);
	else if (FILTER::Support == 1) // point sampling
	{		
		const char* srcData = (char*)_Source.data;
		char* dstData = (char*)_pDestination->data;

		// Bresenham style for x
		int fixedStep = (_SourceInfo.dimensions.x / _DestinationInfo.dimensions.x)*ELEMENT_SIZE;
		int remainder = (_SourceInfo.dimensions.x % _DestinationInfo.dimensions.x);

		for (int y = 0; y < _DestinationInfo.dimensions.y; y++)
		{
			int row = (y*_SourceInfo.dimensions.y)/_DestinationInfo.dimensions.y;
			const char* oRESTRICT srcRow = byte_add(srcData, row*_Source.row_pitch);
			char* oRESTRICT dstRow = byte_add(dstData, y*_pDestination->row_pitch);

			int step = 0;
			for(int x = 0; x < _DestinationInfo.dimensions.x; ++x)
			{
				for (int i = 0;i < ELEMENT_SIZE; ++i)
					*dstRow++ = *(srcRow+i);
				srcRow += fixedStep;
				step += remainder;
				if (step >= _DestinationInfo.dimensions.x)
				{
					srcRow += ELEMENT_SIZE;
					step -= _DestinationInfo.dimensions.x;
				}
			}
		}
	}
	else // have to run a real filter
	{
		if (_SourceInfo.dimensions.x == _DestinationInfo.dimensions.x) //Only need a y filter
			resize_vertical<FILTER, ELEMENT_SIZE>(_SourceInfo, _Source, _DestinationInfo, _pDestination);
		else if (_SourceInfo.dimensions.y == _DestinationInfo.dimensions.y) //Only need a x filter
			resize_horizontal<FILTER, ELEMENT_SIZE>(_SourceInfo, _Source, _DestinationInfo, _pDestination);
		else if (_DestinationInfo.dimensions.x*_SourceInfo.dimensions.y <= _DestinationInfo.dimensions.y*_SourceInfo.dimensions.x) //more efficient to run x filter then y
		{
			info tempInfo = _SourceInfo;
			tempInfo.dimensions.x = _DestinationInfo.dimensions.x;
			tempInfo.layout = surface::layout::tight;
			std::vector<char> tempImage;
			tempImage.resize(total_size(tempInfo));

			mapped_subresource tempMap = get_mapped_subresource(tempInfo, 0, 0, tempImage.data());
			resize_horizontal<FILTER, ELEMENT_SIZE>(_SourceInfo, _Source, tempInfo, &tempMap);
			const_mapped_subresource tempMapConst = tempMap;
			resize_vertical<FILTER, ELEMENT_SIZE>(tempInfo, tempMapConst, _DestinationInfo, _pDestination);
		}
		else // more efficient to run y filter then x
		{
			info tempInfo = _SourceInfo;
			tempInfo.dimensions.y = _DestinationInfo.dimensions.y;
			tempInfo.layout = surface::layout::tight;
			std::vector<char> tempImage;
			tempImage.resize(total_size(tempInfo));

			mapped_subresource tempMap = get_mapped_subresource(tempInfo, 0, 0, tempImage.data());
			resize_vertical<FILTER, ELEMENT_SIZE>(_SourceInfo, _Source, tempInfo, &tempMap);
			const_mapped_subresource tempMapConst = tempMap;
			resize_horizontal<FILTER, ELEMENT_SIZE>(tempInfo, tempMapConst, _DestinationInfo, _pDestination);
		}
	}
}

void resize(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination, filter::value _Filter)
{
	if (_SourceInfo.layout != _DestinationInfo.layout || _SourceInfo.format != _DestinationInfo.format)
		throw std::invalid_argument("incompatible surfaces");

	if (is_block_compressed(_SourceInfo.format))
		throw std::invalid_argument("block compressed formats cannot be resized");

	const int elementSize = element_size(_SourceInfo.format);

	#define FILTER_CASE(filter_type) \
	case filter::filter_type: \
	{	switch (elementSize) \
		{	case 1: resize_internal<Filter<filter_##filter_type>,1>(_SourceInfo, _Source, _DestinationInfo, _pDestination); break; \
			case 2: resize_internal<Filter<filter_##filter_type>,2>(_SourceInfo, _Source, _DestinationInfo, _pDestination); break; \
			case 3: resize_internal<Filter<filter_##filter_type>,3>(_SourceInfo, _Source, _DestinationInfo, _pDestination); break; \
			case 4: resize_internal<Filter<filter_##filter_type>,4>(_SourceInfo, _Source, _DestinationInfo, _pDestination); break; \
			oNODEFAULT; \
		} \
		break; \
	}

	switch (_Filter)
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

void clip(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _pDestination, int2 _SourceOffset)
{
	if (_SourceInfo.layout != _DestinationInfo.layout || _SourceInfo.format != _DestinationInfo.format)
		throw std::invalid_argument("incompatible surfaces");

	if (ouro::surface::is_block_compressed(_SourceInfo.format))
		throw std::invalid_argument("block compressed formats cannot be clipped");

	if (_SourceOffset.x < 0 || _SourceOffset.y < 0)
		throw std::invalid_argument("source offset must be >= 0");

	int2 bottomRight = _SourceOffset + _DestinationInfo.dimensions.xy();
	if (bottomRight.x > _SourceInfo.dimensions.x || bottomRight.y > _SourceInfo.dimensions.y)
		throw std::invalid_argument("_SourceOffset + the dimensions of the destination, must be within the dimensions of the source");

	int elementSize = ouro::surface::element_size(_SourceInfo.format);
	memcpy2d(_pDestination->data, _pDestination->row_pitch, byte_add(_Source.data, _Source.row_pitch*_SourceOffset.y + elementSize*_SourceOffset.x), _Source.row_pitch, _DestinationInfo.dimensions.x*elementSize, _DestinationInfo.dimensions.y);
}

void pad(const info& _SourceInfo, const const_mapped_subresource& _Source, const info& _DestinationInfo, mapped_subresource* _Destination, int2 _DestinationOffset)
{
	if (_SourceInfo.layout != _DestinationInfo.layout || _SourceInfo.format != _DestinationInfo.format)
		throw std::invalid_argument("incompatible surfaces");

	if (is_block_compressed(_SourceInfo.format))
		throw std::invalid_argument("block compressed formats cannot be padded");

	if (any(_DestinationOffset < int2(0,0)))
		throw std::invalid_argument("destination offset must be >= 0");

	int2 bottomRight = _DestinationOffset + _SourceInfo.dimensions.xy();
	if (any(bottomRight > _DestinationInfo.dimensions.xy()))
		throw std::invalid_argument("_DestinationOffset + destination dimensions must be within the dimensions of the source");

	int elementSize = element_size(_SourceInfo.format);
	memcpy2d(byte_add(_Destination->data, _Destination->row_pitch*_DestinationOffset.y + elementSize*_DestinationOffset.x), _Destination->row_pitch, _Source.data, _Source.row_pitch, _SourceInfo.dimensions.x*elementSize, _SourceInfo.dimensions.y);
}

	} // namespace surface
} // namespace ouro
