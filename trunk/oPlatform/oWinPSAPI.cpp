// $(header)
#include "oWinPSAPI.h"
#include <oBasis/oAssert.h>

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
	hPSAPI = oModuleLink("psapi.dll", psapi_dll_functions, (void**)&EnumProcesses, oCOUNTOF(psapi_dll_functions));
	oASSERT(hPSAPI, "");
}

oWinPSAPI::~oWinPSAPI()
{
	oModuleUnlink(hPSAPI);
}
