// $(header)
#include "oWinDWMAPI.h"
#include <oooii/oAssert.h>

static const char* dwmapi_dll_functions[] = 
{
	"DwmDefWindowProc",
	"DwmEnableBlurBehindWindow",
	"DwmEnableComposition",
	"DwmEnableMMCSS",
	"DwmExtendFrameIntoClientArea",
	"DwmFlush",
	"DwmGetColorizationColor",
	"DwmGetCompositionTimingInfo",
	"DwmGetTransportAttributes",
	"DwmGetWindowAttribute",
	"DwmInvalidateIconicBitmaps",
	"DwmIsCompositionEnabled",
	"DwmModifyPreviousDxFrameDuration",
	"DwmQueryThumbnailSourceSize",
	"DwmRegisterThumbnail",
	"DwmSetDxFrameDuration",
	"DwmSetIconicLivePreviewBitmap",
	"DwmSetIconicThumbnail",
	"DwmSetPresentParameters",
	"DwmSetWindowAttribute",
	"DwmUnregisterThumbnail",
	"DwmUpdateThumbnailProperties",
};

oWinDWMAPI::oWinDWMAPI()
{
	hDWM = oModule::Link("Dwmapi.dll", dwmapi_dll_functions, (void**)&DwmDefWindowProc, oCOUNTOF(dwmapi_dll_functions));
	oASSERT(hDWM, "");
}

oWinDWMAPI::~oWinDWMAPI()
{
	oModule::Unlink(hDWM);
}
