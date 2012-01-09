// $(header)
#include <oBasis/oBuffer.h>
#include <oBasis/oColor.h>
#include <oBasis/oLockedPointer.h>
#include <oBasis/oRef.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>

static const char* testImage = "Test/Textures/lena_1.png";

struct TESTImage : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oStringPath path;
		oTESTB(oTestManager::Singleton()->FindFullPath(path.c_str(), testImage), "Failed to find \"%s\"", testImage);

		oStringPath tmp;
		oTESTB(oSystemGetPath(tmp.c_str(), oSYSPATH_TMP), "Failed to find platform temp folder");
		strcat_s(tmp.c_str(), testImage);

		oRef<threadsafe oBuffer> buffer1;
		oTESTB(oBufferCreate(path.c_str(), false, &buffer1), "Load failed: %s", path.c_str());
		oLockedPointer<oBuffer> lockedBuffer1(buffer1);

		oRef<oImage> image1;
		oTESTB(oImageCreate(path.c_str(), lockedBuffer1->GetData(), lockedBuffer1->GetSize(), &image1), "Image create failed: %s", path.c_str());

		oTESTB(oImageSave(image1, tmp, oImage::HIGH_COMPRESSION), "Save failed: %s", tmp);

		oRef<threadsafe oBuffer> buffer2;
		oTESTB(oBufferCreate(tmp, false, &buffer2), "Load failed: %s", tmp);
		oLockedPointer<oBuffer> lockedBuffer2(buffer2);

		// Compare that what we saved is the same as what we loaded
		oTESTB(lockedBuffer1->GetSize() == lockedBuffer2->GetSize(), "Buffer size mismatch");
		oTESTB(!memcmp(lockedBuffer1->GetData(), lockedBuffer2->GetData(), lockedBuffer1->GetSize()), "Save did not write the same bit pattern as was loaded");

		oRef<oImage> image2;
		oTESTB(oImageCreate(path.c_str(), lockedBuffer2->GetData(), lockedBuffer2->GetSize(), &image2), "Image create failed: %s", tmp);

		// Compare that the bits written are the same as the bits read

		const oColor* c1 = (const oColor*)image1->GetData();
		const oColor* c2 = (const oColor*)image2->GetData();

		oImage::DESC i1Desc;
		image1->GetDesc(&i1Desc);	
		const size_t nPixels = image1->GetSize() / sizeof(oColor);
		for (size_t i = 0; i < nPixels; i++)
			oTESTB(c1[i] == c2[i], "Pixel mismatch: %u [%u,%u]", i, i % i1Desc.Dimensions.x, i / i1Desc.Dimensions.x);

	#if 0
		// Not quite ready for this: The API support in FreeImage inside oImage doesn't have this until a later version
		// but when we upgrade, reenable this.
		// Check that loading just the header of the file works
		oImage::DESC descFromHeader;
		oTESTB(oImageCreateDesc(lockedBuffer1->GetData(), lockedBuffer1->GetSize(), &descFromHeader), "Failed to load DESC only");
		oTESTB(!memcmp(&desc, &descFromHeader, sizeof(oImage::DESC)), "Comparison of load full file and load from header failed");
	#endif

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTImage);
