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
#include <oBasis/oBuffer.h>
#include <oStd/color.h>
#include <oBasis/oLockedPointer.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>

static const char* testImage = "Test/Textures/lena_1.png";
static const char* testImageJpg = "Test/Textures/lena_1.jpg";

struct PLATFORM_oImage : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		{
			oStd::path path;
			oTESTB0(FindInputFile(path, testImage));

			oStd::path tmp;
			oTESTB0(BuildPath(tmp, oGetFilebase(testImage), oTest::TEMP));

			oStd::intrusive_ptr<oBuffer> buffer1;
			oTESTB(oBufferLoad(path.c_str(), &buffer1), "Load failed: %s", path.c_str());

			oStd::intrusive_ptr<oImage> image1;
			oTESTB(oImageCreate(path.c_str(), buffer1->GetData(), buffer1->GetSize(), &image1), "Image create failed: %s", path.c_str());

			oTESTB(oImageSave(image1, oImage::UNKNOWN_FILE, oImage::HIGH_COMPRESSION, oImage::DEFAULT, tmp), "Save failed: %s", tmp);

			oStd::intrusive_ptr<oBuffer> buffer2;
			oTESTB(oBufferLoad(tmp, &buffer2), "Load failed: %s", tmp);

			// Compare that what we saved is the same as what we loaded

			oTESTB(buffer1->GetSize() == buffer2->GetSize(), "Buffer size mismatch (orig %u bytes, saved-and-reloaded %u bytes)", buffer1->GetSize(), buffer2->GetSize());
			oTESTB(!memcmp(buffer1->GetData(), buffer2->GetData(), buffer1->GetSize()), "Save did not write the same bit pattern as was loaded");

			oStd::intrusive_ptr<oImage> image2;
			oTESTB(oImageCreate(path.c_str(), buffer2->GetData(), buffer2->GetSize(), &image2), "Image create failed: %s", tmp);

			// Compare that the bits written are the same as the bits read

			const oStd::color* c1 = (const oStd::color*)image1->GetData();
			const oStd::color* c2 = (const oStd::color*)image2->GetData();

			oImage::DESC i1Desc;
			image1->GetDesc(&i1Desc);	
			const size_t nPixels = image1->GetSize() / sizeof(oStd::color);
			for (size_t i = 0; i < nPixels; i++)
				oTESTB(c1[i] == c2[i], "Pixel mismatch: %u [%u,%u]", i, i % i1Desc.Dimensions.x, i / i1Desc.Dimensions.x);

			oImage::DESC descFromHeader;
			oTESTB(oImageGetDesc(buffer1->GetData(), buffer1->GetSize(), &descFromHeader), "Failed to load DESC only");
			oTESTB(!memcmp(&i1Desc, &descFromHeader, sizeof(oImage::DESC)), "Comparison of load full file and load from header failed");
		}

		//TEST jpeg loading
		{
			oStd::path path;
			oTESTB0(FindInputFile(path, testImageJpg));

			oStd::intrusive_ptr<oBuffer> buffer1;
			oTESTB(oBufferLoad(path.c_str(), &buffer1), "Load failed: %s", path.c_str());

			oStd::intrusive_ptr<oImage> image1;
			{
				oStd::scoped_timer timer("***************** free image jpeg load time");
				oTESTB(oImageCreate(path.c_str(), buffer1->GetData(), buffer1->GetSize(), &image1), "Image create failed: %s", path.c_str());
			}

			oTESTI2(image1, 0);
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oImage);
