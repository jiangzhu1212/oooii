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
#include <oPlatform/oVTable.h>

interface VTableFooInterface
{
	virtual bool foo(){ return false; }
	virtual int bar( int a, int b, int c, int d) = 0;
};

struct VTableFoo : public VTableFooInterface
{
	virtual bool foo()
	{
		return true;
	}
	virtual int bar( int a, int b, int c, int d)
	{
		int res = a * b * c *d;
		return res * res;
	}

	int foobar;
};

struct PLATFORM_oVTable : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		static bool RunOnce = false;
		if( RunOnce ) // @oooii-kevin: VTAble patching is bootstrap once per-process operation so we can only test it once per run.
			return SKIPPED;

		RunOnce = true;
		unsigned char temp[1024];
		memset( temp, NULL, 1024 );

		VTableFoo aFoo;
		VTableFooInterface* fooTest = &aFoo;
		oTESTB( oVTableRemap(fooTest, temp, 1024 ) > 0, "Failed to remap VTable");
		oVTablePatch(fooTest);
		fooTest->bar( 2, 3, 3, 2);
		oTESTB(fooTest->foo(), "Call to derived foo() failed");
		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oVTable);