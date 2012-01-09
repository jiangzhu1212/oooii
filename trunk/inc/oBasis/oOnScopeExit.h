// $(header)
// A simple class that calls an oFUNCTION when it goes out of scope. This is 
// useful for using an exit-early-on-failure pattern, but being able to 
// centralize cleanup code without gotos or scope worries.
#pragma once
#ifndef oOnScopeExit_h
#define oOnScopeExit_h

#include <oBasis/oFunction.h>

class oOnScopeExit
{
	oTASK OnDestroy;
public:
	oOnScopeExit(oFUNCTION<void ()> _OnDestroy) : OnDestroy(_OnDestroy) {}
	~oOnScopeExit() { OnDestroy(); }
};

#endif
