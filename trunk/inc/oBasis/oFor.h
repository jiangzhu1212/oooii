// $(header)
// Preview of the new for syntax as best as it can be implemented in VS 2010.
#pragma once
#ifndef oFor_h
#define oFor_h

#include <oBasis/oPlatformFeatures.h>

#ifndef oHAS_AUTO
	#error oFOR not defined for this platform
#endif

// Details of oFOR implementations
#define oFOR_INTERNAL__(_ElementDeclaration, _Container, _InstanceID) \
	bool bLoopOnce##_InstanceID = 0; \
	for (auto oFOREACH_iterator = begin(_Container); oFOREACH_iterator != end(_Container); ++oFOREACH_iterator, bLoopOnce##_InstanceID = false) \
	for (_ElementDeclaration = *oFOREACH_iterator; !bLoopOnce##_InstanceID; bLoopOnce##_InstanceID = true)
#define oFOR_INTERNAL2__(_ElementDeclaration, _Container, _InstanceID) oFOR_INTERNAL__(_ElementDeclaration, _Container, _InstanceID)

// Use this similar to BOOST_FOREACH or C++11's new for syntax. Example:
// oFOR(int x&, writableArrayOfInts)
//	x = 10;
// oFOR(const myobj& x, readableVectorOfObjs)
//	foo += x->GetValue();
#define oFOR(_ElementDeclaration, _Container) oFOR_INTERNAL2__(_ElementDeclaration, _Container, __COUNTER__)

#endif
