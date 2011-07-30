// $(header)
#include <oooii/oooii.h>
#include <oooii/oStandards.h>

void oConsoleReporting::VReport( REPORT_TYPE _Type, const char* _Format, va_list _Args )
{
	static const oColor fg[] = 
	{
		0,
		std::Lime,
		std::White,
		0,
		std::Yellow,
		std::Red,
		std::Yellow,
	};
	oSTATICASSERT(oCOUNTOF(fg) == NUM_REPORT_TYPES);

	static const oColor bg[] = 
	{
		0,
		0,
		0,
		0,
		0,
		0,
		std::Red,
	};
	oSTATICASSERT(oCOUNTOF(fg) == NUM_REPORT_TYPES);

	if (_Type == HEADING)
	{
		char msg[2048];
		vsprintf_s(msg, _Format, _Args);
		oToUpper(msg);
		oConsole::fprintf(stdout, fg[_Type], bg[_Type], msg);
	}
	else
	{
		oConsole::vfprintf(stdout,fg[_Type], bg[_Type], _Format, _Args );
	}
}
