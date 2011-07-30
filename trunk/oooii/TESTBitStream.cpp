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
#include <oooii/oBitStream.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>

struct TESTBitStream : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		std::vector<BYTE> buffer(oKB(4));
		oBitStream bitStream(oGetData(buffer), oGetDataSize(buffer));

		bitStream.Push(5);
		oTESTB(bitStream.CurPos() == 4, "oBitStream failed Push.");
		int i = 4;
		bitStream.Push(i);
		oTESTB(bitStream.CurPos() == 8, "oBitStream failed Push.");

		const char* string = "Test string.";
		bitStream.Push(string);
		oTESTB(bitStream.CurPos() == 22, "oBitStream failed Push.");

		bitStream.Push(1.0f);
		oTESTB(bitStream.CurPos() == 26, "oBitStream failed Push.");

		bitStream.Push((BYTE)'a');
		oTESTB(bitStream.CurPos() == 27, "oBitStream failed Push.");

		bitStream.CurPos(0); // Reset bitStream cursor so it can be read back from the beginning.

		bitStream.Pop(&i);
		oTESTB(i == 5, "Incorrect value returned from oBitStream::Pop<int>.");
		bitStream.Pop(&i);
		oTESTB(i == 4, "Incorrect value returned from oBitStream::Pop<int>.");

		char tempStr[128];
		bitStream.Pop(tempStr);
		oTESTB(0 == strcmp(tempStr, string), "Incorrect string returned from oBitStream::Pop.");

		float f = 0.0f;
		bitStream.Pop(&f);
		oTESTB(f == 1.0f, "Incorrect value returned from oBitStream::Pop<float>.");

		BYTE b;
		bitStream.Pop(&b);
		oTESTB(b == 'a', "Incorrect value returned from oBitStream::Pop<BYTE>.");

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTBitStream);
