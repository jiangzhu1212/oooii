/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#pragma once
#ifndef oWinDWMAPI_h
#define oWinDWMAPI_h

#include "oWinSoftLinkCommon.h"
#include <dwmapi.h>

oDECLARE_DLL_SINGLETON_BEGIN(oWinDWMAPI)
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
oDECLARE_DLL_SINGLETON_END()

#endif
