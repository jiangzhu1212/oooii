// $(header)
#include <oBasis/oNTPTime.h>
#include <oBasis/oError.h>

static const unsigned long long SEC_FROM_1900_TO_1970 = (365ull * 70 + 17) * 24 * 60 * 60;

bool oNTPTimeFromPosix(time_t _PosixTime, oNTPTime* _pNTPTime)
{
	// Posix time is # seconds since Jan 1, 1970 NTP time is # seconds since 
	// Jan 1, 1900 so take the difference. (17 leap years)
	unsigned long long t = _PosixTime + SEC_FROM_1900_TO_1970;
	if (t < static_cast<unsigned long long>(_PosixTime))
		return oErrorSetLast(oERROR_AT_CAPACITY, "Time rollover occurred when converting from Posix to NTP time");
	return true;
}

bool oNTPTimeToPosix(oNTPTime _NTPTime, time_t* _pPosixTime)
{
	if (_NTPTime >= SEC_FROM_1900_TO_1970)
	{
		*_pPosixTime = _NTPTime - SEC_FROM_1900_TO_1970;
		return true;
	}

	return oErrorSetLast(oERROR_AT_CAPACITY, "The specified NTPTime is before the Posix (time_t) epoch, so thus the returned time would be invalid");
}
