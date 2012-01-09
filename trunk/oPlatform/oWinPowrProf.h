// $(header)
#pragma once
#ifndef oWinPowrProf_h
#define oWinPowrProf_h

#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oWindows.h>
#include <PowrProf.h>

struct oWinPowrProf : oModuleSingleton<oWinPowrProf>
{
	oWinPowrProf();
	~oWinPowrProf();

	BOOLEAN (__stdcall *SetSuspendState)(BOOLEAN Hibernate, BOOLEAN ForceCritical, BOOLEAN DisableWakeEvent);

protected:
	oHMODULE hPowrProf;
};

#endif
