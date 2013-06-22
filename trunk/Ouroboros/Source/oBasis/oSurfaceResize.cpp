/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oSurfaceResize.h>
#include <oStd/fixed_vector.h>
#include <oConcurrency/oConcurrency.h>
#include <oBasis/oError.h>
#include <oBasis/oMath.h>
#include <oBasis/oMemory.h>
#include <oBasis/oRTTI.h>
#include <vector>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oSURFACE_FILTER)
	oRTTI_ENUM_BEGIN_VALUES(oSURFACE_FILTER)
		oRTTI_VALUE_CUSTOM(oSURFACE_FILTER_POINT, "point")
		oRTTI_VALUE_CUSTOM(oSURFACE_FILTER_BOX, "box")
		oRTTI_VALUE_CUSTOM(oSURFACE_FILTER_TRIANGLE, "triangle")
		oRTTI_VALUE_CUSTOM(oSURFACE_FILTER_LANCZOS2, "lanczos2")
		oRTTI_VALUE_CUSTOM(oSURFACE_FILTER_LANCZOS3, "lanczos3")
	oRTTI_ENUM_END_VALUES(oSURFACE_FILTER)
oRTTI_ENUM_END_DESCRIPTION(oSURFACE_FILTER)

struct oPointFilter
{
	static const int Width = 0;
protected:

	float GetValue(float _Offset) const
	{
		if (abs(_Offset) < std::numeric_limits<float>::epsilon())
			return 1.0f;
		else
			return 0.0f;
	}
};

struct oBoxFilter
{
	static const int Width = 1;

protected:
	float GetValue(float _Offset) const
	{
		if (abs(_Offset) <= 1.0f)
			return 1.0f;
		else
			return 0.0f;
	}
};

struct oTriangleFilter
{
	static const int Width = 1;

protected:
	float GetValue(float _Offset) const
	{
		float absOffset = abs(_Offset);
		if (absOffset <= 1.0f)
			return 1.0f - absOffset;
		else
			return 0.0f;
	}
};

struct oLanczos2Filter
{
	static const int Width = 2;

protected:
	float GetValue(float _Offset) const
	{
		const float invWidth = 1.0f/Width;
		float absOffset = abs(_Offset);
		if (absOffset <= Width)
			return oSinc(_Offset) * oSinc(_Offset*invWidth);
		else
			return 0.0f;
	}
};

struct oLanczos3Filter
{
	static const int Width = 3;

protected:
	float GetValue(float _Offset) const
	{
		const float invWidth = 1.0f/Width;
		float absOffset = abs(_Offset);
		if (absOffset <= Width)
			return oSinc(_Offset) * oSinc(_Offset*invWidth);
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
		oStd::fixed_vector<float, Support> Cache;
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
				float weight = GetValue(filterLoc);
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
bool oSurfaceResizeHorizontal(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap)
{
	oASSERT(_SrcDesc.Dimensions.y == _DstDesc.Dimensions.y, "Horizontal resize assumes y dimensions are the same for src and dst");

	FILTER filter;
	filter.InitFilter(_SrcDesc.Dimensions.x, _DstDesc.Dimensions.x);

	const char* srcData = (char*)_SrcMap.pData;
	char* dstData = (char*)_DstMap->pData;

	oConcurrency::serial_for(0, _DstDesc.Dimensions.y, [&](size_t _y){
		int y = oInt(_y);
		const uchar* oRESTRICT srcRow = (uchar*)oStd::byte_add(srcData, y*_SrcMap.RowPitch);
		uchar* oRESTRICT dstRow = (uchar*)oStd::byte_add(dstData, y*_DstMap->RowPitch);

		for(int x = 0; x < _DstDesc.Dimensions.x; ++x)
		{
			auto& filterEntry = filter.FilterCache[x];
			float result[ELEMENT_SIZE];
			for (int i = 0;i < ELEMENT_SIZE; ++i)
			{
				result[i] = 0;
			}

			for (int srcX = filterEntry.Left;srcX <= filterEntry.Right; ++srcX)
			{
				for (int i = 0;i < ELEMENT_SIZE; ++i)
				{
					float src = srcRow[srcX*ELEMENT_SIZE + i];
					src *= filterEntry.Cache[srcX - filterEntry.Left];
					result[i] += src;
				}
			}
			for (int i = 0;i < ELEMENT_SIZE; ++i)
			{
				dstRow[x*ELEMENT_SIZE + i] = static_cast<uchar>(clamp(result[i], 0.0f, 255.0f));
			}
		}
	});

	return true;
}

template<typename FILTER, int ELEMENT_SIZE>
bool oSurfaceResizeVertical(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap)
{
	oASSERT(_SrcDesc.Dimensions.x == _DstDesc.Dimensions.x, "Vertical resize assumes x dimensions are the same for src and dst");

	FILTER filter;
	filter.InitFilter(_SrcDesc.Dimensions.y, _DstDesc.Dimensions.y);

	const uchar* oRESTRICT srcData = (uchar*)_SrcMap.pData;
	uchar* oRESTRICT dstData = (uchar*)_DstMap->pData;

	oConcurrency::serial_for(0, _DstDesc.Dimensions.y, [&](size_t _y){
		int y = oInt(_y);
		uchar* oRESTRICT dstRow = oStd::byte_add(dstData, y*_DstMap->RowPitch);

		for(int x = 0; x < _DstDesc.Dimensions.x; ++x)
		{
			auto& filterEntry = filter.FilterCache[y];
			float result[ELEMENT_SIZE];
			for (int i = 0;i < ELEMENT_SIZE; ++i)
			{
				result[i] = 0;
			}

			for (int srcY = filterEntry.Left;srcY <= filterEntry.Right; ++srcY)
			{
				const uchar* oRESTRICT srcElement = oStd::byte_add(srcData, srcY*_SrcMap.RowPitch + x*ELEMENT_SIZE);

				for (int i = 0;i < ELEMENT_SIZE; ++i)
				{
					float src = srcElement[i];
					src *= filterEntry.Cache[srcY - filterEntry.Left];
					result[i] += src;
				}
			}
			for (int i = 0;i < ELEMENT_SIZE; ++i)
			{
				dstRow[x*ELEMENT_SIZE + i] = static_cast<uchar>(clamp(result[i], 0.0f, 255.0f));
			}
		}
	});

	return true;
}

template<typename FILTER, int ELEMENT_SIZE>
bool oSurfaceResize(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap)
{
	//Assuming all our filters are separable for now.

	if (_SrcDesc.Dimensions == _DstDesc.Dimensions) //no actual resize, just copy
	{
		oSurfaceCopySubresource(_SrcDesc, _SrcMap, _DstMap, false);
	}
	else if (FILTER::Support == 1) //point sampling
	{		
		const char* srcData = (char*)_SrcMap.pData;
		char* dstData = (char*)_DstMap->pData;

		//Bresenham style for x
		int fixedStep = (_SrcDesc.Dimensions.x / _DstDesc.Dimensions.x)*ELEMENT_SIZE;
		int remainder = (_SrcDesc.Dimensions.x % _DstDesc.Dimensions.x);

		//Parallel for doesn't help
		oConcurrency::serial_for(0, _DstDesc.Dimensions.y, [&](size_t _y){
			int y = oInt(_y);
			int row = (y*_SrcDesc.Dimensions.y)/_DstDesc.Dimensions.y;
			const char* oRESTRICT srcRow = oStd::byte_add(srcData, row*_SrcMap.RowPitch);
			char* oRESTRICT dstRow = oStd::byte_add(dstData, y*_DstMap->RowPitch);

			int step = 0;
			for(int x = 0; x < _DstDesc.Dimensions.x; ++x)
			{
				for (int i = 0;i < ELEMENT_SIZE; ++i)
				{
					*dstRow++ = *(srcRow+i);
				}
				srcRow += fixedStep;
				step += remainder;
				if (step >= _DstDesc.Dimensions.x)
				{
					srcRow += ELEMENT_SIZE;
					step -= _DstDesc.Dimensions.x;
				}
			}
		});
	}
	else //have to run a real filter
	{
		if (_SrcDesc.Dimensions.x == _DstDesc.Dimensions.x) //Only need a y filter
		{
			return oSurfaceResizeVertical<FILTER, ELEMENT_SIZE>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
		}
		else if (_SrcDesc.Dimensions.y == _DstDesc.Dimensions.y) //Only need a x filter
		{
			return oSurfaceResizeHorizontal<FILTER, ELEMENT_SIZE>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
		}
		else if (_DstDesc.Dimensions.x*_SrcDesc.Dimensions.y <= _DstDesc.Dimensions.y*_SrcDesc.Dimensions.x) //more efficient to run x filter then y
		{
			oSURFACE_DESC tempDesc = _SrcDesc;
			tempDesc.Dimensions.x = _DstDesc.Dimensions.x;
			std::vector<char> tempImage;
			tempImage.resize(oSurfaceCalcSize(tempDesc));

			oSURFACE_MAPPED_SUBRESOURCE tempMap;
			oSurfaceCalcMappedSubresource(tempDesc, 0, 0, tempImage.data(), &tempMap);

			if (!oSurfaceResizeHorizontal<FILTER, ELEMENT_SIZE>(_SrcDesc, _SrcMap, tempDesc, &tempMap))
				return false;

			oSURFACE_CONST_MAPPED_SUBRESOURCE tempMapConst = tempMap;
			if (!oSurfaceResizeVertical<FILTER, ELEMENT_SIZE>(tempDesc, tempMapConst, _DstDesc, _DstMap))
				return false;

			return true;
		}
		else //more efficient to run y filter then x
		{
			oSURFACE_DESC tempDesc = _SrcDesc;
			tempDesc.Dimensions.y = _DstDesc.Dimensions.y;
			std::vector<char> tempImage;
			tempImage.resize(oSurfaceCalcSize(tempDesc));

			oSURFACE_MAPPED_SUBRESOURCE tempMap;
			oSurfaceCalcMappedSubresource(tempDesc, 0, 0, tempImage.data(), &tempMap);

			if (!oSurfaceResizeVertical<FILTER, ELEMENT_SIZE>(_SrcDesc, _SrcMap, tempDesc, &tempMap))
				return false;

			oSURFACE_CONST_MAPPED_SUBRESOURCE tempMapConst = tempMap;
			if (!oSurfaceResizeHorizontal<FILTER, ELEMENT_SIZE>(tempDesc, tempMapConst, _DstDesc, _DstMap))
				return false;

			return true;
		}
	}

	return true;
}

bool oSurfaceResize(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, oSURFACE_FILTER _Filter)
{
	if (_SrcDesc.Layout != _DstDesc.Layout || _SrcDesc.Format != _DstDesc.Format)
		return oErrorSetLast(std::errc::invalid_argument, "Incompatible surfaces provided.");

	if (oSurfaceFormatIsBlockCompressed(_SrcDesc.Format))
		return oErrorSetLast(std::errc::invalid_argument, "Block compressed formats can't be resized");

	switch (_Filter)
	{
		case oSURFACE_FILTER_POINT:
		{
			int elementSize = oSurfaceFormatGetSize(_SrcDesc.Format); //making element size compile time so key loops auto unroll
			switch(elementSize)
			{
				case 1: return oSurfaceResize<Filter<oPointFilter>,1>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 2: return oSurfaceResize<Filter<oPointFilter>,2>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 3: return oSurfaceResize<Filter<oPointFilter>,3>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 4: return oSurfaceResize<Filter<oPointFilter>,4>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
					oNODEFAULT;
			}
			break;
		}
		case oSURFACE_FILTER_BOX:
		{
			int elementSize = oSurfaceFormatGetSize(_SrcDesc.Format); //making element size compile time so key loops auto unroll
			switch(elementSize)
			{
				case 1: return oSurfaceResize<Filter<oBoxFilter>,1>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 2: return oSurfaceResize<Filter<oBoxFilter>,2>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 3: return oSurfaceResize<Filter<oBoxFilter>,3>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 4: return oSurfaceResize<Filter<oBoxFilter>,4>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				oNODEFAULT;
			}
			break;
		}
		case oSURFACE_FILTER_TRIANGLE:
		{
			int elementSize = oSurfaceFormatGetSize(_SrcDesc.Format); //making element size compile time so key loops auto unroll
			switch(elementSize)
			{
				case 1: return oSurfaceResize<Filter<oTriangleFilter>,1>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 2: return oSurfaceResize<Filter<oTriangleFilter>,2>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 3: return oSurfaceResize<Filter<oTriangleFilter>,3>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 4: return oSurfaceResize<Filter<oTriangleFilter>,4>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				oNODEFAULT;
			}
			break;
		}
		case oSURFACE_FILTER_LANCZOS2:
		{
			int elementSize = oSurfaceFormatGetSize(_SrcDesc.Format); //making element size compile time so key loops auto unroll
			switch(elementSize)
			{
				case 1: return oSurfaceResize<Filter<oLanczos2Filter>,1>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 2: return oSurfaceResize<Filter<oLanczos2Filter>,2>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 3: return oSurfaceResize<Filter<oLanczos2Filter>,3>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 4: return oSurfaceResize<Filter<oLanczos2Filter>,4>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				oNODEFAULT;
			}
			break;
		}
		case oSURFACE_FILTER_LANCZOS3:
		{
			int elementSize = oSurfaceFormatGetSize(_SrcDesc.Format); //making element size compile time so key loops auto unroll
			switch(elementSize)
			{
				case 1: return oSurfaceResize<Filter<oLanczos3Filter>,1>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 2: return oSurfaceResize<Filter<oLanczos3Filter>,2>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 3: return oSurfaceResize<Filter<oLanczos3Filter>,3>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				case 4: return oSurfaceResize<Filter<oLanczos3Filter>,4>(_SrcDesc, _SrcMap, _DstDesc, _DstMap);
				oNODEFAULT;
			}
			break;
		}
	}

	return oErrorSetLast(std::errc::invalid_argument, "Unsupported filter type");
}

bool oSurfaceClip(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, int2 _SrcOffset)
{
	if (_SrcDesc.Layout != _DstDesc.Layout || _SrcDesc.Format != _DstDesc.Format)
		return oErrorSetLast(std::errc::invalid_argument, "Incompatible surfaces provided.");

	if (oSurfaceFormatIsBlockCompressed(_SrcDesc.Format))
		return oErrorSetLast(std::errc::invalid_argument, "Block compressed formats can't be clipped");

	if (_SrcOffset.x < 0 || _SrcOffset.y < 0)
		return oErrorSetLast(std::errc::invalid_argument, "src offset must be >= 0");

	int2 bottomRight = _SrcOffset + _DstDesc.Dimensions.xy();
	if (bottomRight.x > _SrcDesc.Dimensions.x || bottomRight.y > _SrcDesc.Dimensions.y)
		return oErrorSetLast(std::errc::invalid_argument, "_srcOffset + the dimensions of the destination, must be within the dimensions of the source");

	int elementSize = oSurfaceFormatGetSize(_SrcDesc.Format);
	oMemcpy2d(_DstMap->pData, _DstMap->RowPitch, oStd::byte_add(_SrcMap.pData, _SrcMap.RowPitch*_SrcOffset.y + elementSize*_SrcOffset.x), _SrcMap.RowPitch, _DstDesc.Dimensions.x*elementSize, _DstDesc.Dimensions.y);

	return true;
}

bool oSurfacePad(const oSURFACE_DESC& _SrcDesc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _SrcMap, const oSURFACE_DESC& _DstDesc, oSURFACE_MAPPED_SUBRESOURCE* _DstMap, int2 _DstOffset)
{
	if (_SrcDesc.Layout != _DstDesc.Layout || _SrcDesc.Format != _DstDesc.Format)
		return oErrorSetLast(std::errc::invalid_argument, "Incompatible surfaces provided.");

	if (oSurfaceFormatIsBlockCompressed(_SrcDesc.Format))
		return oErrorSetLast(std::errc::invalid_argument, "Block compressed formats can't be padded");

	if (_DstOffset.x < 0 || _DstOffset.y < 0)
		return oErrorSetLast(std::errc::invalid_argument, "src offset must be >= 0");

	int2 bottomRight = _DstOffset + _SrcDesc.Dimensions.xy();
	if (bottomRight.x > _DstDesc.Dimensions.x || bottomRight.y > _DstDesc.Dimensions.y)
		return oErrorSetLast(std::errc::invalid_argument, "_srcOffset + the dimensions of the destination, must be within the dimensions of the source");

	int elementSize = oSurfaceFormatGetSize(_SrcDesc.Format);
	oMemcpy2d(oStd::byte_add(_DstMap->pData, _DstMap->RowPitch*_DstOffset.y + elementSize*_DstOffset.x), _DstMap->RowPitch, _SrcMap.pData, _SrcMap.RowPitch, _SrcDesc.Dimensions.x*elementSize, _SrcDesc.Dimensions.y);
	
	return true;
}