// $(header)
#include "oWinPowrProf.h"
#include <oBasis/oAssert.h>

static const char* powrprof_dll_functions[] = 
{
	"SetSuspendState",
};

oWinPowrProf::oWinPowrProf()
{
	hPowrProf = oModuleLink("PowrProf.dll", powrprof_dll_functions, (void**)&SetSuspendState, oCOUNTOF(powrprof_dll_functions));
	oASSERT(hPowrProf, "");
}

oWinPowrProf::~oWinPowrProf()
{
	oModuleUnlink(hPowrProf);
}
