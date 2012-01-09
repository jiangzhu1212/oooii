// $(header)
#pragma once
#ifndef oBasisTestCommon_h
#define oBasisTestCommon_h

#include <oBasis/oError.h>

#define oTESTB(test, msg, ...) do { if (!(test)) return oErrorSetLast(oERROR_GENERIC, msg, ## __VA_ARGS__); } while(false)

#endif
