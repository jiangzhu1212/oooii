// $(header)
#include "oWinPSAPI.h"
#include <oooii/oAssert.h>

static const char* psapi_dll_functions[] = 
{
	"EnumProcesses",
	"EnumProcessModules",
	"GetModuleBaseNameA",
	"GetProcessMemoryInfo",
	"GetModuleInformation",
};

oWinPSAPI::oWinPSAPI()
{
	hPSAPI = oModule::Link("psapi.dll", psapi_dll_functions, (void**)&EnumProcesses, oCOUNTOF(psapi_dll_functions));
	oASSERT(hPSAPI, "");
}

oWinPSAPI::~oWinPSAPI()
{
	oModule::Unlink(hPSAPI);
}
