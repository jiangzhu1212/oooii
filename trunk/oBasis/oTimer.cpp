// $(header)
#include <oBasis/oTimer.h>

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
	if (*pTimeoutMSCountdown != oInfiniteWait)
	{
		unsigned int CurrentTime = oTimerMS();
		unsigned int diff = CurrentTime - Start;
		unsigned int OldCountdown = *pTimeoutMSCountdown;
		*pTimeoutMSCountdown = OldCountdown < diff ? 0 :  OldCountdown - diff;
		Start = CurrentTime;
	}
}
