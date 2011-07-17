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
#include <oooii/oTest.h>
#include <oooii/oooii.h>

static const char* TESTHash_Filename = "lena.png";

struct TESTHash : public oTest
{
	
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		char tmp[_MAX_PATH];
		oGetSysPath(tmp, oSYSPATH_TMP);
		sprintf_s(tmp, "%s%s", tmp, TESTHash_Filename);

		oRef<threadsafe oBuffer> buffer1;
		oTESTB(oBuffer::Create(tmp, false, &buffer1), "Load failed: %s", tmp);
		oLockedPointer<oBuffer> lockedBuffer1(buffer1);

		uint128_t ExpectedHash = {5036109567207105818, 7580512480722386524};
		uint128_t ComputedHash = oHash_murmur3_x64_128( lockedBuffer1->GetData(), static_cast<unsigned int>( lockedBuffer1->GetSize() ) );
		oTESTB(ExpectedHash == ComputedHash, "Hash doesn't match" );

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTHash);