// $(header)
#pragma once
#ifndef oStandards_h
#define oStandards_h

// This contains patterns and standards that programmers at OOOii Realtime follow
// when implementing various products.

namespace oConsoleReporting
{
	enum REPORT_TYPE
	{
		DEFAULT,
		SUCCESS,
		INFO,
		HEADING,
		WARN,
		ERR,
		CRIT,
		NUM_REPORT_TYPES,
	};

	void VReport(REPORT_TYPE _Type, const char* _Format, va_list _Args);
	inline void Report(REPORT_TYPE _Type, const char* _Format, ...) { va_list args; va_start(args, _Format); VReport(_Type, _Format, args); va_end( args ); }
}

#endif // oStandards_h