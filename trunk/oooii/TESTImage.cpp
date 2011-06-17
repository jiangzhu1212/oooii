// $(header)
#include <oooii/oBuffer.h>
#include <oooii/oColor.h>
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

		char FullPath[_MAX_PATH];
		oTESTB(oTestManager::Singleton()->FindFullPath(FullPath, testImage), "Failed to find %s", testImage);

		oRef<threadsafe oBuffer> buffer1;
		oTESTB(oBuffer::Create(FullPath, false, &buffer1), "Load failed: %s", FullPath);
		oLockedPointer<oBuffer> lockedBuffer1(buffer1);

		oRef<oImage> image1;
		oTESTB(oImage::Create(lockedBuffer1->GetData(), lockedBuffer1->GetSize(), &image1), "Image create failed: %s", FullPath);

		oTESTB(image1->Save(tmp, oImage::HIGH), "Save failed: %s", tmp);

		oRef<threadsafe oBuffer> buffer2;
		oTESTB(oBuffer::Create(tmp, false, &buffer2), "Load failed: %s", tmp);
		oLockedPointer<oBuffer> lockedBuffer2(buffer2);

		// Compare that what we saved is the same as what we loaded
		oTESTB(lockedBuffer1->GetSize() == lockedBuffer2->GetSize(), "Buffer size mismatch");
		oTESTB(!memcmp(lockedBuffer1->GetData(), lockedBuffer2->GetData(), lockedBuffer1->GetSize()), "Save did not write the same bit pattern as was loaded");

		oRef<oImage> image2;
		oTESTB(oImage::Create(lockedBuffer2->GetData(), lockedBuffer2->GetSize(), &image2), "Image create failed: %s", tmp);

		// Compare that the bits written are the same as the bits read

		oImage::DESC desc;
		image1->GetDesc(&desc);

		const oColor* c1 = (const oColor*)image1->Map();
		const oColor* c2 = (const oColor*)image2->Map();

		const size_t nPixels = desc.Size / sizeof(oColor);
		for (size_t i = 0; i < nPixels; i++)
			oTESTB(c1[i] == c2[i], "Pixel mismatch: %u [%u,%u]", i, i % desc.Width, i / desc.Width);

		image1->Unmap();
		image2->Unmap();

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTImage);
