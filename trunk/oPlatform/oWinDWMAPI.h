// $(header)
#pragma once
#ifndef oWinDWMAPI_h
#define oWinDWMAPI_h

#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oWindows.h>
#include <dwmapi.h>

struct oWinDWMAPI : oModuleSingleton<oWinDWMAPI>
{
	oHMODULE hDWM;

	oWinDWMAPI();
	~oWinDWMAPI();

public:
	BOOL (__stdcall *DwmDefWindowProc)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);
	HRESULT (__stdcall *DwmEnableBlurBehindWindow)(HWND hWnd, const DWM_BLURBEHIND *pBlurBehind);
	HRESULT (__stdcall *DwmEnableComposition)(UINT uCompositionAction);
	HRESULT (__stdcall *DwmEnableMMCSS)(BOOL fEnableMMCSS);
	HRESULT (__stdcall *DwmExtendFrameIntoClientArea)(HWND hWnd, const MARGINS *pMarInset);
	HRESULT (__stdcall *DwmFlush)(void);
	HRESULT (__stdcall *DwmGetColorizationColor)(DWORD *pcrColorization, BOOL *pfOpaqueBlend);
	HRESULT (__stdcall *DwmGetCompositionTimingInfo)(HWND hwnd, DWM_TIMING_INFO *pTimingInfo);
	HRESULT (__stdcall *DwmGetTransportAttributes)(BOOL *pfIsRemoting, BOOL *pfIsConnected, DWORD *pDwGeneration);
	HRESULT (__stdcall *DwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);
	HRESULT (__stdcall *DwmInvalidateIconicBitmaps)(HWND hwnd);
	HRESULT (__stdcall *DwmIsCompositionEnabled)(BOOL *pfEnabled);
	HRESULT (__stdcall *DwmModifyPreviousDxFrameDuration)(HWND hwnd, INT cRefreshes, BOOL fRelative);
	HRESULT (__stdcall *DwmQueryThumbnailSourceSize)(HTHUMBNAIL hThumbnail, PSIZE pSize);
	HRESULT (__stdcall *DwmRegisterThumbnail)(HWND hwndDestination, HWND hwndSource, PHTHUMBNAIL phThumbnailId);
	HRESULT (__stdcall *DwmSetDxFrameDuration)(HWND hwnd, INT cRefreshes);
	HRESULT (__stdcall *DwmSetIconicLivePreviewBitmap)(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags);
	HRESULT (__stdcall *DwmSetIconicThumbnail)(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags);
	HRESULT (__stdcall *DwmSetPresentParameters)(HWND hwnd, DWM_PRESENT_PARAMETERS *pPresentParams);
	HRESULT (__stdcall *DwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
	HRESULT (__stdcall *DwmUnregisterThumbnail)(HTHUMBNAIL hThumbnailId);
	HRESULT (__stdcall *DwmUpdateThumbnailProperties)(HTHUMBNAIL hThumbnailId, const DWM_THUMBNAIL_PROPERTIES *ptnProperties);
};

#endif
