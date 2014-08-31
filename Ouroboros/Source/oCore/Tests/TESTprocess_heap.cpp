// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/process_heap.h>
#include "../../test_services.h"
#include <oBase/throw.h>

namespace ouro {
	namespace tests {

struct TestStaticContext
{
	TestStaticContext()
		: Counter(1234)
	{}

	int Counter;

	static void Ctor(void* _Memory) { new (_Memory) TestStaticContext(); }
};

void TESTprocess_heap()
{
	TestStaticContext* c = 0;
	bool allocated = process_heap::find_or_allocate(
		sizeof(TestStaticContext)
		, typeid(TestStaticContext).name()
		, process_heap::per_process
		, process_heap::leak_tracked
		, TestStaticContext::Ctor
		, nullptr
		, (void**)&c);
	
	oCHECK(allocated && c && c->Counter == 1234, "Failed to construct context");
	
	c->Counter = 4321;
	allocated = process_heap::find_or_allocate(
		sizeof(TestStaticContext)
		, typeid(TestStaticContext).name()
		, process_heap::per_process
		, process_heap::leak_tracked
		, TestStaticContext::Ctor
		, nullptr
		, (void**)&c);
	oCHECK(!allocated && c && c->Counter == 4321, "Failed to attach context");

	process_heap::deallocate(c);
}

	} // namespace tests
} // namespace ouro
