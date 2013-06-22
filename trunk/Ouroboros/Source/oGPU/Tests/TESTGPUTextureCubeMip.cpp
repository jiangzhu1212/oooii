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

struct GPU_TextureCubeMip : public oGPUTextureTest
{
	virtual enum oGPU_TEST_PIPELINE GetPipeline() override
	{
		return oGPU_TEST_TEXTURE_CUBE;
	}

	virtual bool CreateTexture() override
	{
		oRef<oImage> images[6];
		if (!oImageLoad("file://DATA/Test/Textures/CubePosX.png", oImage::FORCE_ALPHA, &images[0]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/CubeNegX.png", oImage::FORCE_ALPHA, &images[1]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/CubePosY.png", oImage::FORCE_ALPHA, &images[2]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/CubeNegY.png", oImage::FORCE_ALPHA, &images[3]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/CubePosZ.png", oImage::FORCE_ALPHA, &images[4]))
			return false;
		if (!oImageLoad("file://DATA/Test/Textures/CubeNegZ.png", oImage::FORCE_ALPHA, &images[5]))
			return false;

		if (!oGPUCreateTexture(Device, (const oImage**)&images[0], oCOUNTOF(images), oGPU_TEXTURE_CUBE_MAP_MIPS, &Texture))
			return false;

		return true;
	}

	virtual float GetRotationStep() override
	{
		float rotationStep = (Device->GetFrameID()-1) * 1.0f;
		if (Device->GetFrameID()==0)
			rotationStep = 774.0f;
		else if (Device->GetFrameID()==2)
			rotationStep = 1036.0f;
		return rotationStep;
	}
};

oTEST_REGISTER(GPU_TextureCubeMip);