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

struct GPU_Texture3D : public oGPUTextureTest
{
	virtual enum oGPU_TEST_PIPELINE GetPipeline() override
	{
		return oGPU_TEST_TEXTURE_3D;
	}

	virtual bool CreateTexture() override
	{
		oRef<oImage> images[3];
		if (!oImageLoad("file://DATA/Test/Textures/Red.png", oImage::FORCE_ALPHA, &images[0]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/Green.png", oImage::FORCE_ALPHA, &images[1]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/Blue.png", oImage::FORCE_ALPHA, &images[2]))
			return false;

		if (!oGPUCreateTexture(Device, (const oImage**)&images[0], oCOUNTOF(images), oGPU_TEXTURE_3D_MAP, &Texture))
			return false;

		return true;
	}
};

oTEST_REGISTER(GPU_Texture3D);
