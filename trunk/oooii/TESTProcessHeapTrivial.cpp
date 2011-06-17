// $(header)
#include <oooii/oRef.h>
#include <oooii/oBuffer.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oProcessHeap.h>
#include <oooii/oTest.h>

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
		const char* NAME = "TestBuffer";

		TestStaticContext* c = 0;
		bool allocated = oProcessHeap::FindOrAllocate(NAME, sizeof(TestStaticContext), TestStaticContext::Ctor, (void**)&c);
		oTESTB(allocated && c && c->Counter == 1234, "Failed to construct context");
		c->Counter = 4321;

		allocated = oProcessHeap::FindOrAllocate(NAME, sizeof(TestStaticContext), TestStaticContext::Ctor, (void**)&c);
		oTESTB(!allocated && c && c->Counter == 4321, "Failed to attach context");

		oProcessHeap::Deallocate(c);
		return SUCCESS;
	}
};

oTEST_REGISTER(TESTProcessHeapTrivial);
