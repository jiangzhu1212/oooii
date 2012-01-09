// $(header)
// Utilities for working with the Window's Tray
#pragma once
#ifndef oWinTray_h
#define oWinTray_h

#include <oPlatform/oWindows.h>

// Return the HWND of the "system tray" or "notification area"
HWND oTrayGetHwnd();

// Sets the focus on the tray itself, not any icon in it
void oTraySetFocus();

// Get the rectangle of the specified icon. Returns true if the rect is valid, 
// or false if the icon doesn't exist.
bool oTrayGetIconRect(HWND _hWnd, UINT _ID, RECT* _pRect);

// Returns true of the tray icon exists
inline bool oTrayExists(HWND _hWnd, UINT _ID) { RECT r; return oTrayGetIconRect(_hWnd, _ID, &r); }

// Icons are identified by the HWND and the ID. If a CallbackMessage is not 
// zero, then this is a message that can be handled in the HWND's WNDPROC. Use 
// WM_USER+n for the custom code. If HICON is 0, the icon from the HWND will be 
// used. If HICON is valid it will be used. All lifetime management is up to the 
// user.
void oTrayShowIcon(HWND _hWnd, UINT _ID, UINT _CallbackMessage, HICON _hIcon, bool _Show);

// Once an icon has been created with oTrayShowIcon, use this to display a 
// message on it. If _hIcon is null, then the _hWnd's icon is used.
bool oTrayShowMessage(HWND _hWnd, UINT _ID, HICON _hIcon, UINT _TimeoutMS, const char* _Title, const char* _Message);

// Helper function for the WNDPROC that handles the _CallbackMessage specified 
// above.
void oTrayDecodeCallbackMessageParams(WPARAM _wParam, LPARAM _lParam, UINT* _pNotificationEvent, UINT* _pID, int* _pX, int* _pY);

// Minimize a window to the tray (animates a window to the tray)
void oTrayMinimize(HWND _hWnd, UINT _CallbackMessage, HICON _hIcon);

// Animates an existing tray icon to a restored window
void oTrayRestore(HWND _hWnd);

#endif
