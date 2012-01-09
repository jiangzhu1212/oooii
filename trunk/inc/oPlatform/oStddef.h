// $(header)
// Ubiquitous types often found in header/interface definitions are added here
// to keep includes simple. Only add things here that are used very often.
#pragma once
#ifndef oStddef_h
#define oStddef_h

#include <oBasis/oMacros.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oNoncopyable.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oStringize.h>
#include <oBasis/oThreadsafe.h>

#define oDEFAULT 0x80000000

// Constants used throughout the code for asynchronous/time-based operations. Look to 
// comments on an API to understand when it is appropriate to use these.
const static unsigned int oINFINITE_WAIT = oInvalid;

#endif
