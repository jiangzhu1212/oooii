// $(header)
// Network Time Protocol (NTP) http://tools.ietf.org/html/rfc1059
#pragma once
#ifndef oNTPTime_h
#define oNTPTime_h

#include <ctime> // time_t

typedef unsigned long long oNTPTime;

bool oNTPTimeFromPosix(time_t _PosixTime, oNTPTime* _pNTPTime);
bool oNTPTimeToPosix(oNTPTime _NTPTime, time_t* _pPosixTime);

#endif
