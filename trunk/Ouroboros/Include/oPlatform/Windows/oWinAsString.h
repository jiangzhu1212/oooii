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
// Debug utils for converting common Windows enums and messages 
// into strings.
#pragma once
#ifndef oWinAsString_h
#define oWinAsString_h

#include <oPlatform/Windows/oWindows.h>

const char* oWinAsStringHT(unsigned int _HTCode);
const char* oWinAsStringSC(unsigned int _SCCode);
const char* oWinAsStringSW(unsigned int _SWCode);
const char* oWinAsStringWM(unsigned int _uMsg);
const char* oWinAsStringWS(unsigned int _WSFlag);
const char* oWinAsStringWA(unsigned int _WACode);
const char* oWinAsStringBST(unsigned int _BSTCode);
const char* oWinAsStringNM(unsigned int _NMCode);
const char* oWinAsStringSWP(unsigned int _SWPCode);
const char* oWinAsStringGWL(unsigned int _GWLCode);
const char* oWinAsStringGWLP(unsigned int _GWLPCode);
const char* oWinAsStringTCN(unsigned int _TCNCode);
const char* oWinAsStringCDERR(unsigned int _CDERRCode); // common dialog errors
const char* oWinAsStringExceptionCode(unsigned int _ExceptionCode);

char* oWinParseStyleFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSFlags);
char* oWinParseSWPFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _SWPFlags);

// Fills _StrDestination with a string of the WM_* message and details 
// about its parameters. This can be useful for printing out debug details.
char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
inline char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, const CWPSTRUCT* _pCWPStruct) { return oWinParseWMMessage(_StrDestination, _SizeofStrDestination, _pCWPStruct->hwnd, _pCWPStruct->message, _pCWPStruct->wParam, _pCWPStruct->lParam); }

const char* oWinAsStringHR(HRESULT _hResult);
const char* oWinAsStringHR_DXGI(HRESULT _hResult);
const char* oWinAsStringHR_DX11(HRESULT _hResult);
const char* oWinAsStringHR_VFW(HRESULT _hResult);
const char* oWinAsStringDISP(UINT _DISPCode);

bool oWinParseHRESULT(char* _StrDestination, size_t _SizeofStrDestination, HRESULT _hResult);

template<size_t size> inline char* oWinParseStyleFlags(char (&_StrDestination)[size], UINT _WSFlags) { return oWinParseStyleFlags(_StrDestination, size, _WSFlags); }
template<size_t size> inline char* oWinParseSWPFlags(char (&_StrDestination)[size], UINT _SWPFlags) { return oWinParseSWPFlags(_StrDestination, size, _SWPFlags); }
template<size_t size> inline char* oWinParseWMMessage(char (&_StrDestination)[size], HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) { return oWinParseWMMessage(_StrDestination, size, _hWnd, _uMsg, _wParam, _lParam); }
template<size_t size> inline char* oWinParseWMMessage(char (&_StrDestination)[size], const CWPSTRUCT* _pCWPStruct) { return oWinParseWMMessage(_StrDestination, size, _pCWPStruct); }
template<size_t size> inline bool oWinParseHRESULT(char (&_StrDestination)[size], HRESULT _hResult) { return oWinParseHRESULT(_StrDestination, size, _hResult); }
#endif
