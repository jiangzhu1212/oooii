// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oConcurrency/coroutine.h>
#include <oConcurrency/future.h>
#include <thread>
#include "../../test_services.h"

namespace ouro { namespace tests {

class MyContext : public coroutine_context
{
public:
	MyContext() 
		: Flag(false)
		, Done(false)
		, Counter(0)
	{}

	future<void> Future;
	bool Flag;
	bool Done;
	int Counter;
};

void MyExecute(MyContext& _MyContext)
{
	_MyContext.Counter++;
	oCoBegin(&_MyContext);
	_MyContext.Future = ouro::async([&](){ _MyContext.Flag = true; });
	oCoYield(); // this causes MyExecute to return, but it has marked where it has left off
	if (!_MyContext.Future.is_ready())
		return;
	_MyContext.Done = true;
	oCoEnd();
}

void TESTcoroutine(test_services& services)
{
	MyContext myContext;

	while (!myContext.Done)
	{
		MyExecute(myContext);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

}}
