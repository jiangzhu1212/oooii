// $(header)
#include <oBasis/oBuffer.h>
#include <oBasis/oRef.h>
#include <oBasis/oLockedPointer.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oTest.h>

// {0BC98B60-0F99-4688-A028-F49B0271A237}
static const oGUID GUIDTestBuffer = { 0xbc98b60, 0xf99, 0x4688, { 0xa0, 0x28, 0xf4, 0x9b, 0x2, 0x71, 0xa2, 0x37 } };

struct TESTProcessHeapTrivial : public oTest
{
	struct TestStaticContext
	{
		TestStaticContext()
			: Counter(1234)
		{}

		int Counter;

		static void Ctor(void* _Memory) { new (_Memory) TestStaticContext(); }
	};

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		TestStaticContext* c = 0;
		bool allocated = oProcessHeapFindOrAllocate(GUIDTestBuffer, false, true, sizeof(TestStaticContext), TestStaticContext::Ctor, "TestBuffer", (void**)&c);
		oTESTB(allocated && c && c->Counter == 1234, "Failed to construct context");
		c->Counter = 4321;

		allocated = oProcessHeapFindOrAllocate(GUIDTestBuffer, false, true, sizeof(TestStaticContext), TestStaticContext::Ctor, "TestBuffer", (void**)&c);
		oTESTB(!allocated && c && c->Counter == 4321, "Failed to attach context");

		oProcessHeapDeallocate(c);
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTProcessHeapTrivial);
