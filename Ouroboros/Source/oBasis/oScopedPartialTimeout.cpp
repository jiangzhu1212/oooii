// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oScopedPartialTimeout.h>

oScopedPartialTimeout::oScopedPartialTimeout(unsigned int* _pTimeoutMSCountdown)
	: pTimeoutMSCountdown(_pTimeoutMSCountdown)
	, Start(ouro::timer::nowmsi())
{
}

oScopedPartialTimeout::~oScopedPartialTimeout()
{
	UpdateTimeout();
}

void oScopedPartialTimeout::UpdateTimeout()
{
	if (*pTimeoutMSCountdown != ouro::infinite)
	{
		unsigned int CurrentTime = ouro::timer::nowmsi();
		unsigned int diff = CurrentTime - Start;
		unsigned int OldCountdown = *pTimeoutMSCountdown;
		*pTimeoutMSCountdown = OldCountdown < diff ? 0 :  OldCountdown - diff;
		Start = CurrentTime;
	}
}
