/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// Debug utils for converting common Windows enums and messages 
// into strings.
#pragma once
#ifndef oWinAsString_h
#define oWinAsString_h

#include "../Source/oStd/win.h"
#include <oPlatform/Windows/oWinKey.h>

const char* oWinAsStringHT(int _HTCode);
const char* oWinAsStringSC(int _SCCode);
const char* oWinAsStringSW(int _SWCode);
const char* oWinAsStringWM(int _uMsg);
const char* oWinAsStringWS(int _WSFlag);
const char* oWinAsStringWSEX(int _WSEXFlag);
const char* oWinAsStringWA(int _WACode);
const char* oWinAsStringBST(int _BSTCode);
const char* oWinAsStringNM(int _NMCode);
const char* oWinAsStringSWP(int _SWPCode);
const char* oWinAsStringGWL(int _GWLCode);
const char* oWinAsStringGWLP(int _GWLPCode);
const char* oWinAsStringTCN(int _TCNCode);
const char* oWinAsStringCDERR(int _CDERRCode); // common dialog errors
//const char* oWinAsStringExceptionCode(int _ExceptionCode);
const char* oWinAsStringDBT(int _DBTEvent);
const char* oWinAsStringDBTDT(int _DBTDevType);
const char* oWinAsStringSPDRP(int _SPDRPValue);

char* oWinParseStyleFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSFlags);
char* oWinParseStyleExFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSEXFlags);
char* oWinParseSWPFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _SWPFlags);

// Fills _StrDestination with a string of the WM_* message and details 
// about its parameters. This can be useful for printing out debug details.
char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, oWINKEY_CONTROL_STATE* _pState, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
inline char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, oWINKEY_CONTROL_STATE* _pState, const CWPSTRUCT* _pCWPStruct) { return oWinParseWMMessage(_StrDestination, _SizeofStrDestination, _pState, _pCWPStruct->hwnd, _pCWPStruct->message, _pCWPStruct->wParam, _pCWPStruct->lParam); }

const char* oWinAsStringHR(HRESULT _hResult);
const char* oWinAsStringHR_DXGI(HRESULT _hResult);
const char* oWinAsStringHR_DX11(HRESULT _hResult);
const char* oWinAsStringHR_VFW(HRESULT _hResult);
const char* oWinAsStringDISP(UINT _DISPCode);

bool oWinParseHRESULT(char* _StrDestination, size_t _SizeofStrDestination, HRESULT _hResult);

template<size_t size> inline char* oWinParseStyleFlags(char (&_StrDestination)[size], UINT _WSFlags) { return oWinParseStyleFlags(_StrDestination, size, _WSFlags); }
template<size_t size> inline char* oWinParseStyleExFlags(char (&_StrDestination)[size], UINT _WSEXFlags) { return oWinParseStyleExFlags(_StrDestination, size, _WSEXFlags); }
template<size_t size> inline char* oWinParseSWPFlags(char (&_StrDestination)[size], UINT _SWPFlags) { return oWinParseSWPFlags(_StrDestination, size, _SWPFlags); }
template<size_t size> inline char* oWinParseWMMessage(char (&_StrDestination)[size], HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) { return oWinParseWMMessage(_StrDestination, size, _hWnd, _uMsg, _wParam, _lParam); }
template<size_t size> inline char* oWinParseWMMessage(char (&_StrDestination)[size], const CWPSTRUCT* _pCWPStruct) { return oWinParseWMMessage(_StrDestination, size, _pCWPStruct); }
template<size_t size> inline bool oWinParseHRESULT(char (&_StrDestination)[size], HRESULT _hResult) { return oWinParseHRESULT(_StrDestination, size, _hResult); }
#endif
