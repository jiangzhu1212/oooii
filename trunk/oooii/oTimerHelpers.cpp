// $(header)
#include <oooii/oTimerHelpers.h>
#include <oooii/oAssert.h>
#include <oooii/oStdio.h>

using namespace std::tr1;

oScopedPartialTimeout::oScopedPartialTimeout(unsigned int* _pTimeoutMSCountdown)
	: pTimeoutMSCountdown(_pTimeoutMSCountdown)
	, Start(oTimerMS())
{
}

oScopedPartialTimeout::~oScopedPartialTimeout()
{
	UpdateTimeout();
}

void oScopedPartialTimeout::UpdateTimeout()
{
	if (*pTimeoutMSCountdown != oINFINITE_WAIT)
	{
		unsigned int CurrentTime = oTimerMS();
		unsigned int diff = CurrentTime - Start;
		unsigned int OldCountdown = *pTimeoutMSCountdown;
		*pTimeoutMSCountdown = OldCountdown < diff ? 0 :  OldCountdown - diff;
		Start = CurrentTime;
	}
}

oScopedTraceTimer::oScopedTraceTimer(const char* _Name) : Name(_Name)
{
	Start = oTimer();
}

oScopedTraceTimer::~oScopedTraceTimer()
{
	double delta = oTimer() - Start;
	oTRACEA("%s took %.03f sec", Name, delta);
}
