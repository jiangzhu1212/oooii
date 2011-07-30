// $(header)
#include <oooii/oTest.h>
#include <oooii/oVTable.h>

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

struct TESTVTable : public oTest
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

oTEST_REGISTER(TESTVTable);