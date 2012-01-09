// $(header)
// Debug utils for converting common Windows enums and messages 
// into strings.
#pragma once
#ifndef oWinAsString_h
#define oWinAsString_h

#include <oPlatform/oWindows.h>

const char* oWinAsStringHT(unsigned int _HTCode);
const char* oWinAsStringSC(unsigned int _SCCode);
const char* oWinAsStringSW(unsigned int _SWCode);
const char* oWinAsStringWM(unsigned int _uMsg);
const char* oWinAsStringWS(unsigned int _WSFlag);
const char* oWinAsStringWA(unsigned int _WACode);

char* oWinParseStyleFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSFlags);
char* oWinParseSWPFlags(char* _StrDestination, size_t _SizeofStrDestination, UINT _SWPFlags);

// Fills _StrDestination with a string of the WM_* message and details 
// about its parameters. This can be useful for printing out debug details.
char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
inline char* oWinParseWMMessage(char* _StrDestination, size_t _SizeofStrDestination, const CWPSTRUCT* _pCWPStruct) { return oWinParseWMMessage(_StrDestination, _SizeofStrDestination, _pCWPStruct->hwnd, _pCWPStruct->message, _pCWPStruct->wParam, _pCWPStruct->lParam); }

const char* oWinAsStringHR_DXGI(HRESULT _hResult);
const char* oWinAsStringHR_VFW(HRESULT _hResult);
const char* oWinAsStringDISP(UINT _DISPCode);

bool oWinParseHRESULT(char* _StrDestination, size_t _SizeofStrDestination, HRESULT _hResult);

template<size_t size> inline char* oWinParseStyleFlags(char (&_StrDestination)[size], UINT _WSFlags) { return oWinParseStyleFlags(_StrDestination, size, _WSFlags); }
template<size_t size> inline char* oWinParseSWPFlags(char (&_StrDestination)[size], UINT _SWPFlags) { return oWinParseSWPFlags(_StrDestination, size, _SWPFlags); }
template<size_t size> inline char* oWinParseWMMessage(char (&_StrDestination)[size], HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) { return oWinParseWMMessage(_StrDestination, size, _hWnd, _uMsg, _wParam, _lParam); }
template<size_t size> inline char* oWinParseWMMessage(char (&_StrDestination)[size], const CWPSTRUCT* _pCWPStruct) { return oWinParseWMMessage(_StrDestination, size, _pCWPStruct); }
template<size_t size> inline bool oWinParseHRESULT(char (&_StrDestination)[size], HRESULT _hResult) { return oWinParseHRESULT(_StrDestination, size, _hResult); }
#endif
