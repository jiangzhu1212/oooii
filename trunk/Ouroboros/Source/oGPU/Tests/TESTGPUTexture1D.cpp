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
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/oGPUUtil.h>

struct GPU_Texture1D : public oGPUTextureTest
{
	virtual enum oGPU_TEST_PIPELINE GetPipeline() override
	{
		return oGPU_TEST_TEXTURE_1D;
	}

	virtual bool CreateTexture() override
	{
		oImage::DESC imageDesc;
		imageDesc.Dimensions = int2(512, 1);
		imageDesc.Format = oImage::BGRA32;
		imageDesc.RowPitch = oImageCalcRowPitch(imageDesc.Format, imageDesc.Dimensions.x);
		oRef<oImage> image;
		oImageCreate("GPU_Texture1D", imageDesc, &image);

		oRef<oBuffer> buffer;
		int surfaceBufferSize = oImageCalcSize(imageDesc.Format, imageDesc.Dimensions);
		void *pSurfaceBuffer = oBuffer::New(surfaceBufferSize);
		oBufferCreate("GPU_Texture1D buffer", pSurfaceBuffer, surfaceBufferSize, oBuffer::Delete, &buffer);

		static const oStd::color sConsoleColors[] = { oStd::Black, oStd::Navy, oStd::Green, oStd::Teal, oStd::Maroon, oStd::Purple, oStd::Olive, oStd::Silver, oStd::Gray, oStd::Blue, oStd::Lime, oStd::Aqua, oStd::Red, oStd::Fuchsia, oStd::Yellow, oStd::White };

		oStd::color* texture1Ddata = (oStd::color*)buffer->GetData();
		for (int i=0; i<imageDesc.Dimensions.x; ++i)
		{
			texture1Ddata[i] = sConsoleColors[i % oCOUNTOF(sConsoleColors)];
		}

		image->CopyData(buffer->GetData(), imageDesc.RowPitch);

		if (!oGPUCreateTexture(Device, &image, 1, oGPU_TEXTURE_1D_MAP, &Texture))
			return false;

		return true;
	}

};

oTEST_REGISTER(GPU_Texture1D);
