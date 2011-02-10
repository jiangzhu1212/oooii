/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include "pch.h"
#include <oooii/oBuffer.h>
#include <oooii/oImage.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oPath.h>
#include <oooii/oRef.h>
#include <oooii/oTest.h>

static const char* testImage = "lena.png";

struct TESTImage : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char tmp[_MAX_PATH];
		oGetSysPath(tmp, oSYSPATH_TMP);
		sprintf_s(tmp, "%s%s", tmp, testImage);

		threadsafe oRef<oBuffer> buffer1;
		oTESTB(oBuffer::Create(testImage, false, &buffer1), "Load failed: %s", testImage);
		oLockedPointer<oBuffer> lockedBuffer1(buffer1);

		oRef<oImage> image1;
		oTESTB(oImage::Create(lockedBuffer1->GetData(), lockedBuffer1->GetSize(), &image1), "Image create failed: %s", testImage);

		oTESTB(image1->Save(tmp, oImage::HIGH), "Save failed: %s", tmp);

		threadsafe oRef<oBuffer> buffer2;
		oTESTB(oBuffer::Create(tmp, false, &buffer2), "Load failed: %s", tmp);
		oLockedPointer<oBuffer> lockedBuffer2(buffer2);

		// Compare that what we saved is the same as what we loaded
		oTESTB(lockedBuffer1->GetSize() == lockedBuffer2->GetSize(), "Buffer size mismatch");
		oTESTB(!memcmp(lockedBuffer1->GetData(), lockedBuffer2->GetData(), lockedBuffer1->GetSize()), "Save did not write the same bit pattern as was loaded");

		oRef<oImage> image2;
		oTESTB(oImage::Create(lockedBuffer2->GetData(), lockedBuffer2->GetSize(), &image2), "Image create failed: %s", tmp);

		// Compare that the bits written are the same as the bits read

		const oColor* c1 = (const oColor*)image1->GetData();
		const oColor* c2 = (const oColor*)image2->GetData();

		oImage::DESC desc;
		image1->GetDesc(&desc);

		const size_t nPixels = desc.Size / sizeof(oColor);
		for (size_t i = 0; i < nPixels; i++)
			oTESTB(c1[i] == c2[i], "Pixel mismatch: %u [%u,%u]", i, i % desc.Width, i / desc.Width);

		return SUCCESS;
	}
};

TESTImage TestImage;
