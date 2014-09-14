// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Debug utils for converting common Windows enums and messages 
// into strings.
#pragma once
#ifndef oGUI_win_as_string_h
#define oGUI_win_as_string_h

#include <oGUI/Windows/oWinKey.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ouro {
	namespace windows {
		namespace as_string {

const char* HT(int _HTCode);
const char* SC(int _SCCode);
const char* SW(int _SWCode);
const char* WM(int _uMsg);
const char* WS(int _WSFlag);
const char* WSEX(int _WSEXFlag);
const char* WA(int _WACode);
const char* BST(int _BSTCode);
const char* NM(int _NMCode);
const char* SWP(int _SWPCode);
const char* GWL(int _GWLCode);
const char* GWLP(int _GWLPCode);
const char* TCN(int _TCNCode);
const char* CDERR(int _CDERRCode); // common dialog errors
const char* DBT(int _DBTEvent);
const char* DBTDT(int _DBTDevType);
const char* SPDRP(int _SPDRPValue);

char* style_flags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSFlags);
template<size_t size> inline char* style_flags(char (&_StrDestination)[size], UINT _WSFlags) { return style_flags(_StrDestination, size, _WSFlags); }

char* style_ex_flags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSEXFlags);
template<size_t size> inline char* style_ex_flags(char (&_StrDestination)[size], UINT _WSEXFlags) { return style_ex_flags(_StrDestination, size, _WSEXFlags); }

char* swp_flags(char* _StrDestination, size_t _SizeofStrDestination, UINT _SWPFlags);
template<size_t size> inline char* swp_flags(char (&_StrDestination)[size], UINT _SWPFlags) { return swp_flags(_StrDestination, size, _SWPFlags); }

		} // namespace as_string

// Fills _StrDestination with a string of the WM_* message and details 
// about its parameters. This can be useful for printing out debug details.
char* parse_wm_message(char* _StrDestination, size_t _SizeofStrDestination, oWINKEY_CONTROL_STATE* _pState, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam);
template<size_t size> inline char* parse_wm_message(char (&_StrDestination)[size], HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam) { return parse_wm_message(_StrDestination, size, _hWnd, _uMsg, _wParam, _lParam); }

inline char* parse_wm_message(char* _StrDestination, size_t _SizeofStrDestination, oWINKEY_CONTROL_STATE* _pState, const CWPSTRUCT* _pCWPStruct) { return parse_wm_message(_StrDestination, _SizeofStrDestination, _pState, _pCWPStruct->hwnd, _pCWPStruct->message, _pCWPStruct->wParam, _pCWPStruct->lParam); }
template<size_t size> inline char* parse_wm_message(char (&_StrDestination)[size], const CWPSTRUCT* _pCWPStruct) { return parse_wm_message(_StrDestination, size, _pCWPStruct); }

	}
}

#endif
