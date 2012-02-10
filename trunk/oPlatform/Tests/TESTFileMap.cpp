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

#include <oPlatform/oFile.h>
#include <oPlatform/oTest.h>

struct TESTFileMap : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static const char* testPath = "Test/Textures/lena_1.png";
		
		oStringPath path;
		oTESTB0(FindInputFile(path, testPath));

		oFileRange r;
		{
			oFILE_DESC FDesc;
			oTESTB0(oFileGetDesc(path, &FDesc));
			r.Offset = 0;
			r.Size = FDesc.Size;
		}

		void* mapped = nullptr;
		oTESTB0(oFileMap(path, true, r, &mapped));
		oOnScopeExit OSEUnmap([&] { if (mapped) oFileUnmap(mapped); }); // safety unmap if we fail for some non-mapping reason

		void* loaded = nullptr;
		size_t loadedSize = 0;
		oTESTB0(oFileLoad(&loaded, &loadedSize, malloc, path));
		oOnScopeExit OSEFreeLoadedBuffer([&] { if (loaded) free(loaded); });

		oTESTB(r.Size == loadedSize, "mismatch: mapped and loaded file sizes");
		oTESTB(!memcmp(loaded, mapped, r.Size), "memcmp failed between mapped and loaded files");
		oTESTB0(oFileUnmap(mapped));
		mapped = nullptr; // signal OSEUnmap to not re-unmap

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTFileMap);
