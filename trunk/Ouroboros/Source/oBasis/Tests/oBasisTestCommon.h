// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBasisTestCommon_h
#define oBasisTestCommon_h

#include <oBasis/oError.h>

#define oTESTB0(test) do { if (!(test)) return false; } while(false) // pass through error
#define oTESTB(test, msg, ...) do { if (!(test)) return oErrorSetLast(std::errc::protocol_error, msg, ## __VA_ARGS__); } while(false)

#endif
