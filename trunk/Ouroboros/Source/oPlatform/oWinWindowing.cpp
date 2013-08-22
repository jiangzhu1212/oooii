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
#include <oPlatform/Windows/oWinWindowing.h>
#include <oStd/assert.h>
#include <oStd/fixed_string.h>
#include <oStd/oStdChrono.h>
#include <oConcurrency/mutex.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinStatusBar.h>
#include "SoftLink/oWinCommCtrl.h"
#include "SoftLink/oWinShlwapi.h"
#include <WindowsX.h>
#include <CdErr.h>
#include <Shellapi.h>

#define oWIN_CHECK(_hWnd) do \
	{	if (!oWinExists(_hWnd)) return oErrorSetLast(std::errc::invalid_argument, "Invalid HWND 0x%x specified", _hWnd); \
		if (!oWinIsWindowThread(_hWnd)) return oErrorSetLast(std::errc::operation_not_permitted, "This function must be called on the window thread %d for HWND 0x%x", oConcurrency::asuint(oStd::this_thread::get_id()), _hWnd); \
	} while (false)

#define oWIN_CHECK0(_hWnd) do \
	{	if (!oWinExists(_hWnd)) { oErrorSetLast(std::errc::invalid_argument, "Invalid HWND 0x%x specified", _hWnd); return 0; } \
		if (!oWinIsWindowThread(_hWnd)) { oErrorSetLast(std::errc::operation_not_permitted, "This function must be called on the window thread %d for HWND 0x%x", oConcurrency::asuint(oStd::this_thread::get_id()), _hWnd); return 0; } \
	} while (false)

static const char* kRegisteredWindowMessages[] = 
{
	"oWM_INPUT_DEVICE_STATUS",
};
static_assert(oCOUNTOF(kRegisteredWindowMessages) == oWM_REGISTERED_COUNT, "array mismatch");

const char* oWinGetMessageRegisterString(oWM _RegisteredMessage)
{
	if (_RegisteredMessage < oWM_REGISTERED_FIRST) return nullptr;
	return kRegisteredWindowMessages[_RegisteredMessage - oWM_REGISTERED_FIRST];
}

struct oWinRegisteredMessageContext : oProcessSingleton<oWinRegisteredMessageContext>
{
	static const oGUID GUID;
	oWinRegisteredMessageContext()
	{
		oFORI(i, kRegisteredWindowMessages)
			RegisteredMessages[i] = RegisterWindowMessage(kRegisteredWindowMessages[i]);
	}

	UINT FindMessage(UINT _uMsg)
	{
		oFORI(i, RegisteredMessages)
			if (RegisteredMessages[i] == _uMsg)
				return (UINT)(oWM_REGISTERED_FIRST + i);
		return 0;
	}

	UINT RegisteredMessages[oWM_REGISTERED_COUNT];
};

// {F2F6803E-B2F3-41F3-8C1A-9EA7ADB1EB90}
const oGUID oWinRegisteredMessageContext::GUID = { 0xf2f6803e, 0xb2f3, 0x41f3, { 0x8c, 0x1a, 0x9e, 0xa7, 0xad, 0xb1, 0xeb, 0x90 } };

oSINGLETON_REGISTER(oWinRegisteredMessageContext);

struct oSkeletonInputContext : oProcessSingleton<oSkeletonInputContext>
{
	static const oGUID GUID;

	oSkeletonInputContext() {}
	~oSkeletonInputContext()
	{
		oASSERT(Sources.empty(), "");
	}

	void Register(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _Get)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		if (Sources.find(_hSkeleton) != Sources.end())
			throw std::exception("redundant registration");
		Sources[_hSkeleton] = _Get;
	}

	void Unregister(HSKELETON _hSkeleton)
	{
		oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it != Sources.end())
			Sources.erase(it);
	}

	bool Get(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton)
	{
		// prevent Unregister() from being called during a read
		oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it == Sources.end())
			return oErrorSetLast(std::errc::invalid_argument);
		it->second(_pSkeleton);
		return true;
	}

	bool Close(HSKELETON _hSkeleton)
	{
		return true;
	}

	oConcurrency::shared_mutex Mutex;
	std::map<HSKELETON
		, oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>
		, std::less<HSKELETON>
		, oProcessHeapAllocator<std::pair<HSKELETON, oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>>>> 
	Sources;
};

// {7E5F5608-2C7A-43E7-B7A0-46293FEC653C}
const oGUID oSkeletonInputContext::GUID = { 0x7e5f5608, 0x2c7a, 0x43e7, { 0xb7, 0xa0, 0x46, 0x29, 0x3f, 0xec, 0x65, 0x3c } };

oSINGLETON_REGISTER(oSkeletonInputContext);

void RegisterSkeletonSource(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _Get)
{
	oSkeletonInputContext::Singleton()->Register(_hSkeleton, _Get);
}

void UnregisterSkeletonSource(HSKELETON _hSkeleton)
{
	oSkeletonInputContext::Singleton()->Unregister(_hSkeleton);
}

bool GetSkeletonDesc(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton)
{
	return oSkeletonInputContext::Singleton()->Get(_hSkeleton, _pSkeleton);
}

struct oWIN_DEVICE_CHANGE_CONTEXT
{
	oWIN_DEVICE_CHANGE_CONTEXT()
	{
		LastChangeTimestamp.fill(0);
	}

	std::vector<RAWINPUTDEVICELIST> RawInputs;
	std::vector<oStd::mstring> RawInputInstanceNames;
	std::array<unsigned int, oGUI_INPUT_DEVICE_TYPE_COUNT> LastChangeTimestamp;
};

// Register the specified window to receive WM_INPUT_DEVICE_CHANGE events. This
// also uses oWM_INPUT_DEVICE_CHANGE to emulate the on-creation events fired for
// non-KB/mouse devices like the Kinect.
static bool oWinRegisterDeviceChangeEvents(HWND _hWnd)
{
	RAWINPUTDEVICE RID[] =
	{
		{ oUS_USAGE_PAGE_GENERIC_DESKTOP, oUS_USAGE_KEYBOARD, RIDEV_DEVNOTIFY, _hWnd },
		{ oUS_USAGE_PAGE_GENERIC_DESKTOP, oUS_USAGE_MOUSE, RIDEV_DEVNOTIFY, _hWnd },
	};
	if (!RegisterRawInputDevices(RID, oCOUNTOF(RID), sizeof(RAWINPUTDEVICE)))
		return oWinSetLastError();

	// Windows sends KB and mouse device notifications automatically, so make up the
	// difference here.
	oVERIFY_R(oWinEnumInputDevices(false, [&](const oWINDOWS_HID_DESC& _HIDDesc)
	{
		switch(_HIDDesc.Type)
		{
			case oGUI_INPUT_DEVICE_KEYBOARD: case oGUI_INPUT_DEVICE_MOUSE: case oGUI_INPUT_DEVICE_UNKNOWN: break;
			default: SendMessage(_hWnd, oWM_INPUT_DEVICE_CHANGE, MAKEWPARAM(_HIDDesc.Type, oGUI_INPUT_DEVICE_READY), (LPARAM)_HIDDesc.DeviceInstancePath.c_str());
		}
	}));

	return true;
}

// There's a lot of code to parse a device change message, so encapsulate it 
// here. Basically when devices are removed, there's no name passed to the 
// event of what was removed, so basically a record of all devices needs to be 
// kept and then scanned for differences. That's what the context is. Useage 
// should be to create a context for a window that will support 
// WM_INPUT_DEVICE_CHANGE and then call oWinTranslateDeviceChange in that event
// to translate it into the simpler oWM_INPUT_DEVICE_CHANGE message. Remember to 
// call oWinRegisterDeviceChangeEvents, else the WM_INPUT_DEVICE_CHANGE event 
// will not be fired and this hook will not execute.
oDECLARE_HANDLE(HDEVICECHANGE)
static HDEVICECHANGE oWinDeviceChangeCreate()
{
	return (HDEVICECHANGE)new oWIN_DEVICE_CHANGE_CONTEXT();
}

static void oWinDeviceChangeDestroy(HDEVICECHANGE _hDeviceChance)
{
	oWIN_DEVICE_CHANGE_CONTEXT* ctx = (oWIN_DEVICE_CHANGE_CONTEXT*)_hDeviceChance;
	delete ctx;
}

static oGUI_INPUT_DEVICE_TYPE oWinGetTypeFromRIM(DWORD _dwRIMType)
{
	switch (_dwRIMType)
	{
		case RIM_TYPEKEYBOARD: return oGUI_INPUT_DEVICE_KEYBOARD;
		case RIM_TYPEMOUSE: return oGUI_INPUT_DEVICE_MOUSE;
		default: break;
	}
	return oGUI_INPUT_DEVICE_UNKNOWN;
}

static bool oWinTranslateDeviceChange(HWND _hWnd, WPARAM _wParam, LPARAM _lParam, HDEVICECHANGE _hDeviceChance)
{
	oWIN_DEVICE_CHANGE_CONTEXT* ctx = (oWIN_DEVICE_CHANGE_CONTEXT*)_hDeviceChance;
	oGUI_INPUT_DEVICE_TYPE Type = oGUI_INPUT_DEVICE_UNKNOWN;
	oGUI_INPUT_DEVICE_STATUS Status = _wParam == GIDC_ARRIVAL ? oGUI_INPUT_DEVICE_READY : oGUI_INPUT_DEVICE_NOT_READY;
	oStd::mstring InstanceName;

	switch (_wParam)
	{
		// We have full and direct information.
		case GIDC_ARRIVAL:
		{
			RID_DEVICE_INFO RIDDI;
			RIDDI.cbSize = sizeof(RIDDI);
			UINT Size = sizeof(RIDDI);
			UINT NameCapacity = oSizeT(InstanceName.capacity());
			oVB(GetRawInputDeviceInfoA((HANDLE)_lParam, RIDI_DEVICEINFO, &RIDDI, &Size));
			oVB(GetRawInputDeviceInfoA((HANDLE)_lParam, RIDI_DEVICENAME, InstanceName.c_str(), &NameCapacity));

			// Refresh record keeping in case these are new devices
			UINT RawCapacity = 0;
			GetRawInputDeviceList(nullptr, &RawCapacity, sizeof(RAWINPUTDEVICELIST));
			ctx->RawInputs.resize(RawCapacity);
			ctx->RawInputInstanceNames.resize(RawCapacity);
			UINT RawCount = GetRawInputDeviceList(ctx->RawInputs.data(), &RawCapacity, sizeof(RAWINPUTDEVICELIST));
			for (UINT i = 0; i < RawCount; i++)
			{
				UINT Capacity = oSizeT(ctx->RawInputInstanceNames[i].capacity());
				oVB(GetRawInputDeviceInfoA(ctx->RawInputs[i].hDevice, RIDI_DEVICENAME, ctx->RawInputInstanceNames[i].c_str(), &Capacity));
			}

			oASSERT(RawCount == RawCapacity, "size mismatch");
			Type = oWinGetTypeFromRIM(RIDDI.dwType);
			break;
		}

		// Handle is correct, but device info won't be, not even type, so
		// look it up in app bookkeeping.
		case GIDC_REMOVAL:
		{
			for (size_t i = 0; i < ctx->RawInputs.size(); i++)
			{
				if (ctx->RawInputs[i].hDevice == (HANDLE)_lParam)
				{
					Type = oWinGetTypeFromRIM(ctx->RawInputs[i].dwType);
					InstanceName = ctx->RawInputInstanceNames[i];
					break;
				}
			}

			break;
		}
		oNODEFAULT;
	}

	const unsigned int Timestamp = (unsigned int)GetMessageTime();;
	if (ctx->LastChangeTimestamp[Type] != Timestamp)
	{
		ctx->LastChangeTimestamp[Type] = Timestamp;
		SendMessage(_hWnd, oWM_INPUT_DEVICE_CHANGE, MAKEWPARAM(Type, Status), (LPARAM)InstanceName.c_str());
	}

	return true;
}

// @oooii-tony: Confirmation of hWnd being on the specified thread is disabled
// for now... I think it might be trying to access the HWND before it's fully
// constructed. First get the massive integration done, then come back to this.

#define oWINVP(_hWnd) \
	if (!oWinExists(_hWnd)) \
		{ oErrorSetLast(std::errc::invalid_argument, "Invalid HWND %p specified", _hWnd); return nullptr; } \
	oASSERT(oWinIsWindowThread(_hWnd), "This function must be called on the window thread %d for %p", oConcurrency::asuint(oStd::this_thread::get_id()), _hWnd)

inline bool oErrorSetLastBadType(HWND _hControl, oGUI_CONTROL_TYPE _Type) { return oErrorSetLast(std::errc::invalid_argument, "The specified %s %p (%d) is not valid for this operation", oStd::as_string(_Type), _hControl, GetDlgCtrlID(_hControl)); }

inline bool oErrorSetLastBadIndex(HWND _hControl, oGUI_CONTROL_TYPE _Type, int _SubItemIndex) { return oErrorSetLast(std::errc::invalid_argument, "_SubItemIndex %d was not found in %s %p (%d)", _SubItemIndex, oStd::as_string(_Type), _hControl, GetDlgCtrlID(_hControl)); }

inline HWND oWinControlGetBuddy(HWND _hControl) { return (HWND)SendMessage(_hControl, UDM_GETBUDDY, 0, 0); }

static DWORD oWinGetStyle(oGUI_WINDOW_STYLE _Style, bool _HasParent)
{
	switch (_Style)
	{
		case oGUI_WINDOW_BORDERLESS: return _HasParent ? WS_CHILD : WS_POPUP;
		case oGUI_WINDOW_DIALOG: return WS_CAPTION;
		case oGUI_WINDOW_FIXED: 
		case oGUI_WINDOW_FIXED_WITH_MENU:
		case oGUI_WINDOW_FIXED_WITH_STATUSBAR:
		case oGUI_WINDOW_FIXED_WITH_MENU_AND_STATUSBAR: return WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
		default: break;
	}
	return WS_OVERLAPPEDWINDOW;
}

// Returns the height of the status bar if the specified window has one. If no
// status bar is present or the status bar is hidden, this returns 0.
static int oWinGetStatusBarHeight(HWND _hWnd)
{
	int h = 0;
	if (oWinStatusBarShown(_hWnd))
	{
		RECT rStatusBar;
		HWND hStatusBar = oWinGetStatusBar(_hWnd);
		GetClientRect(hStatusBar, &rStatusBar);
		h = oWinRectH(rStatusBar);
	}
	return h;
}

struct oWndExtra
{
	HMENU hMenu;
	HWND hStatusBar;
	HDEVICECHANGE hDeviceChange;
	LONG_PTR RestoredPosition; // MAKELPARAM(x,y)
	LONG_PTR RestoredSize; // MAKELPARAM(w,h)
	LONG_PTR PreviousState; // oGUI_WINDOW_STATE
	LONG_PTR PreviousStyle; // oGUI_WINDOW_STYLE
	LONG_PTR ExtraFlags;
	intptr_t TempCallCounter;
	intptr_t LastShowTimestamp;
};

// This is set when the handler should ignore the event because its about to be 
// followed up with the same event with more meaningful data.
#define oWNDEXTRA_FLAGS_TEMP_CHANGE (0x1)

#define oGWLP_MENU (offsetof(oWndExtra, hMenu))
#define oGWLP_STATUSBAR (offsetof(oWndExtra, hStatusBar))
#define oGWLP_DEVICE_CHANGE (offsetof(oWndExtra, hDeviceChange))
#define oGWLP_RESTORED_POSITION (offsetof(oWndExtra, RestoredPosition)) // use GET_X_LPARAM and GET_Y_LPARAM to decode
#define oGWLP_RESTORED_SIZE (offsetof(oWndExtra, RestoredSize)) // use GET_X_LPARAM and GET_Y_LPARAM to decode
#define oGWLP_PREVIOUS_STATE (offsetof(oWndExtra, PreviousState)) // oGUI_WINDOW_STATE
#define oGWLP_PREVIOUS_STYLE (offsetof(oWndExtra, PreviousStyle)) // oGUI_WINDOW_STYLE
#define oGWLP_EXTRA_FLAGS (offsetof(oWndExtra, ExtraFlags))
#define oGWLP_TEMP_CALL_COUNTER (offsetof(oWndExtra, TempCallCounter))
#define oGWLP_LAST_SHOW_TIMESTAMP (offsetof(oWndExtra, LastShowTimestamp))

#define oEF_RENDER_TARGET (1<<0)
#define oEF_FULLSCREEN_EXCLUSIVE (1<<1)
#define oEF_FULLSCREEN_COOPERATIVE (1<<2)
#define oEF_NO_SAVE_RESTORED_POSITION_SIZE (1<<3)
#define oEF_ALT_F4_ENABLED (1<<4)

HWND oWinCreate(HWND _hParent
	, const char* _Title
	, oGUI_WINDOW_STYLE _Style
	, const int2& _ClientPosition
	, const int2& _ClientSize
	, WNDPROC _Wndproc
	, void* _pInit
	, void* _pThis)
{
	oStd::sstring ClassName;
	WNDCLASSEXA wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = _Wndproc ? _Wndproc : DefWindowProc;
	// for "native" support for menus and status bars and somewhere to store 
	// restored pos/size
	wc.cbWndExtra = sizeof(oWndExtra);
	wc.lpszClassName = (LPCSTR)oWinMakeClassName(ClassName, _Wndproc);
	wc.style = CS_BYTEALIGNCLIENT|CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	if (0 == RegisterClassEx(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
	{
		oWinSetLastError();
		return nullptr;
	}

	// Resolve initial position and size
	int2 NewPosition = _ClientPosition;
	int2 NewSize = _ClientSize;
	if (NewPosition.x == oDEFAULT || NewPosition.y == oDEFAULT || NewSize.x == oDEFAULT || NewSize.y == oDEFAULT)
	{
		oDISPLAY_DESC dd;
		oDisplayEnum(oDisplayGetPrimaryIndex(), &dd);
		const RECT rPrimaryWorkarea = oWinRectWH(dd.WorkareaPosition, dd.WorkareaSize);
		const int2 DefaultSize = oWinRectSize(rPrimaryWorkarea) / 4; // 25% of parent window by default
		NewSize = oGUIResolveRectSize(NewSize, DefaultSize);
		const RECT rCenteredClient = oWinRect(oGUIResolveRect(oRect(rPrimaryWorkarea), int2(0, 0), NewSize, oGUI_ALIGNMENT_MIDDLE_CENTER, false));
		const int2 DefaultPosition = oWinRectPosition(rCenteredClient);
		NewPosition = oGUIResolveRectPosition(NewPosition, DefaultPosition);
	}

	const DWORD dwInitialStyleEx = WS_EX_ACCEPTFILES|WS_EX_APPWINDOW;
	const DWORD dwInitialStyle = oWinGetStyle(_Style, false);

	RECT rWindow = oWinRectWH(NewPosition, NewSize);
	if (!AdjustWindowRectEx(&rWindow, dwInitialStyle, false, dwInitialStyleEx))
	{
		oWinSetLastError();
		return nullptr;
	}

	oWIN_CREATESTRUCT wcs;
	wcs.Shape.State = oGUI_WINDOW_HIDDEN;
	wcs.Shape.Style = _Style;
	wcs.Shape.ClientPosition = NewPosition;
	wcs.Shape.ClientSize = NewSize;
	wcs.pThis = _pThis;
	wcs.pInit = _pInit;

	// catch any exception thrown by a user's WM_CREATE since oGUI/oWindow passes
	// const events around and thus doesn't know what gets returned.
	HWND hWnd = nullptr;
	try
	{
		hWnd = CreateWindowEx(dwInitialStyleEx, ClassName, oSAFESTRN(_Title), dwInitialStyle
			, rWindow.left, rWindow.top, oWinRectW(rWindow), oWinRectH(rWindow)
			, _hParent, nullptr, nullptr, &wcs);
	}

	catch (std::exception& e)
	{
		oErrorSetLast(e);
		return nullptr;
	}

	if (!hWnd)
	{
		if (GetLastError() == S_OK)
			oErrorSetLast(std::errc::protocol_error, "CreateWindowEx returned a null HWND (failure condition) for '%s' but GetLastError is S_OK. This implies that user handling of a WM_CREATE message failed, so start looking there.", oSAFESTRN(_Title));
		else
			oWinSetLastError();
		return nullptr;
	}

	// does this belong in WM_CREATE?
	oFORI(i, kRegisteredWindowMessages)
		oVB(RegisterWindowMessage(kRegisteredWindowMessages[i]));

	oTRACE("HWND 0x%x '%s' running on thread %d (0x%x)"
		, hWnd
		, oSAFESTRN(_Title)
		, oConcurrency::asuint(oStd::this_thread::get_id())
		, oConcurrency::asuint(oStd::this_thread::get_id()));

	oVERIFY(oWinRegisterDeviceChangeEvents(hWnd));

	return hWnd;
}

void oWinDestroy(HWND _hWnd)
{
	if (oWinIsWindowThread(_hWnd))
		DestroyWindow(_hWnd);
	else
		PostMessage(_hWnd, oWM_DESTROY, 0, 0);
}

char* oWinMakeClassName(char* _StrDestination, size_t _SizeofStrDestination, WNDPROC _Wndproc)
{
	int written = snprintf(_StrDestination, _SizeofStrDestination, "Ouroboros.Window.WndProc.%x", _Wndproc);
	return oSizeT(written) < _SizeofStrDestination ? _StrDestination : nullptr;
}

bool oWinIsClass(HWND _hWnd, WNDPROC _Wndproc)
{
	oStd::sstring ClassName, ExpectedClassName;
	oVERIFY_R(oWinMakeClassName(ExpectedClassName, _Wndproc));
	oVERIFY_R(GetClassName(_hWnd, ClassName.c_str(), oInt(ClassName.capacity())));
	return !strcmp(ClassName, ExpectedClassName);
}

bool oWinSetTempChange(HWND _hWnd, bool _IsTemp)
{
	oWIN_CHECK(_hWnd);
	intptr_t Counter = (intptr_t)GetWindowLongPtr(_hWnd, oGWLP_TEMP_CALL_COUNTER);
	Counter = __max(0, Counter + (_IsTemp ? 1 : -1));
	SetWindowLongPtr(_hWnd, oGWLP_TEMP_CALL_COUNTER, (LONG_PTR)Counter);
	return true;
}

bool oWinIsTempChange(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	intptr_t Counter = (intptr_t)GetWindowLongPtr(_hWnd, oGWLP_TEMP_CALL_COUNTER);
	return Counter != 0;
}

static void oWinSaveRestoredPosSize(HWND _hWnd)
{
	if (!IsIconic(_hWnd) && !IsZoomed(_hWnd) && !oWinIsTempChange(_hWnd))
	{
		LONG_PTR extra = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
		if (0 == (extra & (oEF_FULLSCREEN_COOPERATIVE|oEF_FULLSCREEN_COOPERATIVE|oEF_NO_SAVE_RESTORED_POSITION_SIZE)))
		{
			oGUI_WINDOW_SHAPE_DESC s = oWinGetShape(_hWnd);
			SetWindowLongPtr(_hWnd, oGWLP_RESTORED_POSITION, (LONG_PTR)MAKELPARAM(s.ClientPosition.x, s.ClientPosition.y));
			SetWindowLongPtr(_hWnd, oGWLP_RESTORED_SIZE, (LONG_PTR)MAKELPARAM(s.ClientSize.x, s.ClientSize.y));
			}
	}
}

LRESULT CALLBACK oWinWindowProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if (_hWnd)
	{
		switch (_uMsg)
		{
			case WM_ERASEBKGND:
				// if a render target, don't erase with GDI and cause flickering
				if (oWinIsRenderTarget(_hWnd))
					return 1;
				break;

			case WM_CREATE:
			{
				// 'this' pointer was passed during the call to CreateWindow, so put 
				// that in userdata.
				CREATESTRUCTA* cs = (CREATESTRUCTA*)_lParam;
				oWIN_CREATESTRUCT* wcs = (oWIN_CREATESTRUCT*)cs->lpCreateParams;

				// Set up internal/custom data.
				SetWindowLongPtr(_hWnd, oGWLP_DEVICE_CHANGE, (LONG_PTR)oWinDeviceChangeCreate());
				SetWindowLongPtr(_hWnd, oGWLP_MENU, (LONG_PTR)CreateMenu());
				SetWindowLongPtr(_hWnd, oGWLP_STATUSBAR, (LONG_PTR)oWinStatusBarCreate(_hWnd, (HMENU)0x00005747));
				oVERIFY(oWinShowStatusBar(_hWnd, oGUIStyleHasStatusBar(wcs->Shape.Style)));

				if (oGUIStyleHasStatusBar(wcs->Shape.Style))
					oVB(SetWindowPos(_hWnd, 0, cs->x, cs->y, cs->cx, cs->cy + oWinGetStatusBarHeight(_hWnd), SWP_NOZORDER|SWP_FRAMECHANGED|SWP_NOMOVE));

				oWinSaveRestoredPosSize(_hWnd);
				oWinSetTempChange(_hWnd, wcs->Shape.State != oGUI_WINDOW_MAXIMIZED);
				oWinShowMenu(_hWnd, oGUIStyleHasMenu(wcs->Shape.Style));
				oWinSetTempChange(_hWnd, false);

				SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)wcs->pThis);
				break;
			}

			case WM_INITDIALOG:
				// dialogs don't use CREATESTRUCT, so assume it's directly the context
				SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)_lParam);
				break;

			case WM_DESTROY:
				
				// If GetMenu returns the menu, then Windows will handle its destruction
				if (!::GetMenu(_hWnd))
					DestroyMenu((HMENU)SetWindowLongPtr(_hWnd, oGWLP_MENU, (LONG_PTR)nullptr));

				// As a child window, the status bar is implicitly destroyed.

				// minimizes "randomly set focus to some window other than parent"
				// @oooii-tony: Revisit this: this causes a child window to be reset in 
				// position and looks ugly. It may be true that position needs to be 
				// set here as well.
				//oWinSetOwner(_hWnd, nullptr);

				oWinDeviceChangeDestroy((HDEVICECHANGE)GetWindowLongPtr(_hWnd, oGWLP_DEVICE_CHANGE));
				SetWindowLongPtr(_hWnd, oGWLP_DEVICE_CHANGE, (LONG_PTR)nullptr);

				PostQuitMessage(0);
				break;

			case WM_SYSKEYDOWN:
			{
				if (_wParam == VK_F4 && !oWinAltF4IsEnabled(_hWnd))
					return true;
				break;
			}

			case WM_SYSCOMMAND:
			{
				// Take over control of syscommands to resize since oWinSetShape does
				// extra work DefWindowProc is not aware of.

				oGUI_WINDOW_SHAPE_DESC s;
				switch (_wParam)
				{
					case SC_MAXIMIZE:
						s.State = oGUI_WINDOW_MAXIMIZED;
						oWinSetShape(_hWnd, s);
						return 0;
					case SC_MINIMIZE:
						s.State = oGUI_WINDOW_MINIMIZED;
						oWinSetShape(_hWnd, s);
						return 0;
					case SC_RESTORE:
					{
						oGUI_WINDOW_SHAPE_DESC Old = oWinGetShape(_hWnd);
						if (Old.State == oGUI_WINDOW_MINIMIZED)
						{
							s.State = (oGUI_WINDOW_STATE)GetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STATE);
							if (s.State == oGUI_WINDOW_HIDDEN)
								s.State = oGUI_WINDOW_RESTORED;
						}
						else 
							s.State = oGUI_WINDOW_RESTORED;

						oWinSetShape(_hWnd, s);
						return 0;
					}
					default:
						break;
				}

				break;
			}

			case WM_WINDOWPOSCHANGED:
			{
				if (IsWindowVisible(_hWnd))
					SetWindowLongPtr(_hWnd, oGWLP_LAST_SHOW_TIMESTAMP, (ULONG_PTR)oTimerMS());
				else
					SetWindowLongPtr(_hWnd, oGWLP_LAST_SHOW_TIMESTAMP, (ULONG_PTR)-1);
				break;
			}

			case WM_MOVE:
				oWinSaveRestoredPosSize(_hWnd);
				break;

			case WM_SIZE:
			{
				HWND hStatusBar = (HWND)GetWindowLongPtr(_hWnd, oGWLP_STATUSBAR);
				SendMessage(hStatusBar, WM_SIZE, 0, 0);
				oWinSaveRestoredPosSize(_hWnd);
				break;
			}

			case WM_INPUT_DEVICE_CHANGE:
				oVB(oWinTranslateDeviceChange(_hWnd, _wParam, _lParam, (HDEVICECHANGE)GetWindowLongPtr(_hWnd, oGWLP_DEVICE_CHANGE)));
				break;

			default:
				break;
		}
	}
	return -1;
}

void* oWinGetThis(HWND _hWnd)
{
	oWIN_CHECK0(_hWnd);
	return (void*)GetWindowLongPtr(_hWnd, GWLP_USERDATA);
}

// Returns true if the specified _uMsg is one that was assigned to this process
// by a call to RegisterWindowMessage.
static bool oWinIsRegisteredMessage(UINT _uMsg) { return _uMsg >= 0xC000 && _uMsg <= 0xFFFF; }

UINT oWinTranslateMessage(UINT _uMsg)
{
	if (oWinIsRegisteredMessage(_uMsg))
		return oWinRegisteredMessageContext::Singleton()->FindMessage(_uMsg);
	return _uMsg;
}

oStd::thread::id oWinGetWindowThread(HWND _hWnd)
{
	oStd::thread::id ID;
	(uint&)ID = GetWindowThreadProcessId(_hWnd, nullptr);
	return ID;
}

bool oWinIsOpaque(HWND _hWnd)
{
	// @oooii-tony: I can't find API to ask about the opacity of an HWND, so just
	// wait for a while.
	oWIN_CHECK(_hWnd);
	static const intptr_t kFadeInTime = 200;
	intptr_t LastShowTimestamp = (intptr_t)GetWindowLongPtr(_hWnd, oGWLP_LAST_SHOW_TIMESTAMP);
	if (LastShowTimestamp < 0)
		return false;
	intptr_t Now = oTimerMS();
	return (LastShowTimestamp + kFadeInTime) < Now;
}

bool oWinRegisterTouchEvents(HWND _hWnd, bool _Registered)
{
	#ifdef oWINDOWS_HAS_REGISTERTOUCHWINDOW
		oWINDOWS_VERSION v = oGetWindowsVersion();
		if (v >= oWINDOWS_7)
		{
			if (_Registered)
				oVB(RegisterTouchWindow(_hWnd, 0));
			else
				oVB(UnregisterTouchWindow(_hWnd));
			return true;
		}
		else
	#endif
		return oErrorSetLast(std::errc::not_supported, "Windows 7 is the minimum required version for touch support");
}

void oWinAccelFromHotKeys(ACCEL* _pAccels, const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		ACCEL& a = _pAccels[i];
		const oGUI_HOTKEY_DESC_NO_CTOR& h = _pHotKeys[i];
		a.fVirt = FVIRTKEY;
		if (h.AltDown) a.fVirt |= FALT;
		if (h.CtrlDown) a.fVirt |= FCONTROL;
		if (h.ShiftDown) a.fVirt |= FSHIFT;
		a.key = oWinKeyFromKey(h.HotKey) & 0xffff;
		a.cmd = h.ID;
	}
}

void oWinAccelToHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, const ACCEL* _pAccels, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		const ACCEL& a = _pAccels[i];
		oGUI_HOTKEY_DESC_NO_CTOR& h = _pHotKeys[i];
		oASSERT(a.fVirt & FVIRTKEY, "");
		oWINKEY_CONTROL_STATE dummy;
		h.HotKey = oWinKeyToKey(a.key);
		h.AltDown = !!(a.fVirt & FALT);
		h.CtrlDown = !!(a.fVirt & FCONTROL);
		h.ShiftDown = !!(a.fVirt & FSHIFT);
		h.ID = a.cmd;
	}
}

// A wrapper for MS's GetWindow() that changes the rules of traversal. The rules 
// for the "next" window are: _hWnd's first child, if not then the next 
// sibling, and if at end of sibling list then the first child of _hWnd's parent 
// (what should be the first sibling in _hWnd's sibling list. If the next window 
// has a "buddy" window (UPDOWN control), then the buddy becomes the next, since 
// it controls the events of the other buddy anyway.
static HWND oWinGetNextWindow(HWND _hWnd)
{
	oWINVP(_hWnd);
	HWND hNext = GetWindow(_hWnd, GW_CHILD);
	if (!hNext)
		hNext = GetWindow(_hWnd, GW_HWNDNEXT);
	if (!hNext) // try wrap around
	{
		HWND hParent;
		hParent = GetParent(_hWnd);
		if (!GetParent(hParent)) // if is top-level window bounce back down to first child
			hNext = GetWindow(hParent, GW_CHILD);
		else
			hNext = GetWindow(hParent, GW_HWNDNEXT);

		// handle case where there is exactly one control
		if (_hWnd == hNext)
			return nullptr;
	}

	if (hNext)
	{
		HWND hBuddy = oWinControlGetBuddy(hNext);
		if (hBuddy)
			hNext = hBuddy;
	}
	return hNext;
}

// Returns the last HWND in the specified _hWnd's child list. If not a parent, 
// then this returns nullptr.
static HWND oWinGetLastChild(HWND _hWnd)
{
	oWINVP(_hWnd);
	HWND hChild = GetWindow(_hWnd, GW_CHILD);
	if (hChild)
	{
		HWND hLastChild = GetWindow(hChild, GW_HWNDLAST);
		if (hLastChild) hChild = hLastChild;
	}
	return hChild;
}

// A wrapper for MS's GetWindow() that changes the rules of traversal. The rules
// for the "prev" window are: _hWnd's previous sibling, if at start of sibling 
// list, then get the last child of _hWnd, else get the last child of _hWnd's
// parent.
static HWND oWinGetPrevWindow(HWND _hWnd)
{
	oWINVP(_hWnd);
	HWND hPrev = GetWindow(_hWnd, GW_HWNDPREV);
	if (!hPrev)
	{
		hPrev = GetParent(_hWnd);
		if (hPrev && !GetParent(hPrev)) // don't pop up to top-level window, stay in controls
			hPrev = nullptr;
	}

	else
	{
		HWND hLastChild = oWinGetLastChild(hPrev);
		if (hLastChild) hPrev = hLastChild;
	}

	if (!hPrev) // try wrap around
	{
		hPrev = GetWindow(_hWnd, GW_HWNDLAST);
		HWND hLastChild = oWinGetLastChild(hPrev);
		if (hLastChild) hPrev = hLastChild;
	}

	return hPrev;
}

// Cycles through all next windows until the next valid tabstop is found
static HWND oWinGetNextTabStop()
{
	HWND hCurrent = GetFocus();
	HWND hNext = oWinGetNextWindow(hCurrent);
	while (hNext != hCurrent && hNext && !oWinControlIsTabStop(hNext))
		hNext = oWinGetNextWindow(hNext);
	return hNext;
}

// Cycles through all prev windows until the prev valid tabstop is found
static HWND oWinGetPrevTabStop()
{
	HWND hCurrent = GetFocus();
	HWND hPrev = oWinGetPrevWindow(hCurrent);
	while (hPrev != hCurrent && hPrev && !oWinControlIsTabStop(hPrev))
		hPrev = oWinGetPrevWindow(hPrev);
	return hPrev;
}

// No Wisdom from Google has yielded a rational Microsoft-way of handling 
// Ctrl-Tab and Shift-Ctrl-Tab in a way consistent with Microsoft-system 
// dialogs. This function wraps IsDialogMessage in a way that seems to conform
// more to MS standards than their own function by intercepting WM_KEYDOWN for
// VK_TAB.
static bool oIsDialogMessageEx(HWND _hWnd, MSG* _pMsg)
{
	oWIN_CHECK(_hWnd);
	if (_pMsg->message == WM_KEYDOWN && _pMsg->wParam == VK_TAB)
	{
		bool CtrlDown = (GetKeyState(VK_LCONTROL) & 0x1000) || (GetKeyState(VK_RCONTROL) & 0x1000);
		bool ShiftDown = (GetKeyState(VK_LSHIFT) & 0x1000) || (GetKeyState(VK_RSHIFT) & 0x1000);
		if (CtrlDown)
		{
			// @oooii-tony: Hmm, this won't behave well with multiple tab controls in
			// a single window
			HWND hTab = FindWindowEx(_hWnd, nullptr, WC_TABCONTROL, nullptr);
			while (hTab && (!IsWindowEnabled(hTab) || !IsWindowVisible(hTab)))
				hTab = FindWindowEx(_hWnd, hTab, WC_TABCONTROL, nullptr);

			if (hTab)
			{
				NMHDR n;
				n.hwndFrom = hTab;
				n.idFrom = GetDlgCtrlID(hTab);
				n.code = TCN_SELCHANGING;
				// @oooii-tony: Not sure what this should be because the docs say it 
				// isn't the case that n.idFrom is the ID to use, but I always observe 
				// the values being the same in debug spew
				WPARAM w = n.idFrom;
				SendMessage(_hWnd, WM_NOTIFY, w, (LPARAM)&n);
				oWinControlSelectSubItemRelative(hTab, ShiftDown ? -1 : 1);
				n.code = TCN_SELCHANGE;
				SendMessage(_hWnd, WM_NOTIFY, w, (LPARAM)&n);
				return true;
			}
		}

		else
		{
			HWND hNext = ShiftDown ? oWinGetPrevTabStop() : oWinGetNextTabStop();
			if (hNext)
			{
				oWinSetFocus(hNext);
				return true;
			}
		}
	}

	return !!IsDialogMessage(_hWnd, _pMsg);
}

bool oWinDispatchMessage(HWND _hWnd, HACCEL _hAccel, bool _WaitForNext)
{
	MSG msg;
	bool HasMessage = false;
	if (_WaitForNext)
	{
		int n = GetMessage(&msg, nullptr, 0, 0);
		if (n == 0)
			return oErrorSetLast(std::errc::operation_canceled);
		HasMessage = n > 0;
	}
	else
		HasMessage = !!PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);

	if (HasMessage)
	{
			if (!_hWnd)
				_hWnd = msg.hwnd;

			oWIN_CHECK(_hWnd);

			HasMessage = !!TranslateAccelerator(_hWnd, _hAccel, &msg);
			if (!HasMessage)
				HasMessage = oIsDialogMessageEx(_hWnd, &msg);

			if (!HasMessage)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// Treat WM_NULL like the noop it is by indicating that there are no new messages
			if (msg.message != WM_NULL)
				return true;
		}

	return oErrorSetLast(std::errc::no_message_available);
}

HMENU oWinGetMenu(HWND _hWnd)
{
	oWIN_CHECK0(_hWnd);
	return (HMENU)GetWindowLongPtr(_hWnd, oGWLP_MENU);
}

bool oWinShowMenu(HWND _hWnd, bool _Show)
{
	oWIN_CHECK(_hWnd);
	HMENU hMenu = oWinGetMenu(_hWnd);
	oASSERT(hMenu, "invalid menu");

	// If the top-level menu is empty, then not even the bar gets drawn, and the
	// client area is resized. So in this case, fail out and preserve the sizing.
	if (0 == GetMenuItemCount(hMenu))
		return oErrorSetLast(std::errc::operation_not_permitted, "empty top-level windows do not draw anything, and thus the client rectangle is inappropriately calculated so don't show empty menus");

	if (!::SetMenu(_hWnd, _Show ? hMenu : nullptr))
		return oWinSetLastError();
	return true;
}

bool oWinMenuShown(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	return !!GetMenu(_hWnd);
}

HWND oWinGetStatusBar(HWND _hWnd)
{
	oWIN_CHECK0(_hWnd);
	return (HWND)GetWindowLongPtr(_hWnd, oGWLP_STATUSBAR);
}

bool oWinShowStatusBar(HWND _hWnd, bool _Show)
{
	oWIN_CHECK(_hWnd);
	HWND hStatusBar = oWinGetStatusBar(_hWnd);
	oVERIFY_R(hStatusBar);
	if (!ShowWindow(hStatusBar, _Show ? SW_SHOWNOACTIVATE : SW_HIDE))
		if (!RedrawWindow(hStatusBar, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW))
			return oWinSetLastError();
	return true;
}

bool oWinStatusBarShown(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	HWND hStatusBar = oWinGetStatusBar(_hWnd);
	if (hStatusBar)
		return !!(GetWindowLongPtr(hStatusBar, GWL_STYLE) & WS_VISIBLE);
	return false;
}

bool oWinSetOwner(HWND _hWnd, HWND _hOwner)
{
	// Yes, you've read this right... Check the community add-ons:
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633591(v=vs.85).aspx
	SetLastError(S_OK);
	if (!SetWindowLongPtr(_hWnd, GWLP_HWNDPARENT, (LONG_PTR)_hOwner) && GetLastError() != S_OK)
		return oWinSetLastError();
	return true;
}

HWND oWinGetOwner(HWND _hWnd)
{
	return GetWindow(_hWnd, GW_OWNER);
}

bool oWinSetParent(HWND _hWnd, HWND _hParent)
{
	oWIN_CHECK(_hWnd);
	if (_hParent)
	{
		DWORD dwStyle = (DWORD)GetWindowLongPtr(_hWnd, GWL_STYLE);
		dwStyle = (dwStyle & ~WS_POPUP) | WS_CHILD;
		SetWindowLongPtr(_hWnd, GWL_STYLE, dwStyle);
	}

	::SetParent(_hWnd, _hParent);

	if (!_hParent)
	{
		DWORD dwStyle = (DWORD)GetWindowLongPtr(_hWnd, GWL_STYLE);
		dwStyle = (dwStyle & ~WS_CHILD) | WS_POPUP;
		SetWindowLongPtr(_hWnd, GWL_STYLE, dwStyle);
	}

	return true;
}

HWND oWinGetParent(HWND _hWnd)
{
	return ::GetParent(_hWnd);
}

bool oWinIsParent(HWND _hWnd)
{
	HWND hChild = GetWindow(_hWnd, GW_CHILD);
	return !!hChild;
}

struct OWNER_CTX
{
	OWNER_CTX(HWND _hOwner) : hOwner(_hOwner), IsOwner(false) {}
	HWND hOwner;
	bool IsOwner;
};

static BOOL CALLBACK CheckIsOwner(HWND _hWnd, LPARAM _lParam)
{
	// if there's even one child, then this is a parent.
	OWNER_CTX& ctx = *(OWNER_CTX*)_lParam;
	HWND hOwner = GetWindow(_hWnd, GW_OWNER);
	if (hOwner == ctx.hOwner)
		ctx.IsOwner = true;
	return !ctx.IsOwner;
}

bool oWinIsOwner(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	OWNER_CTX ctx(_hWnd);
	EnumWindows(CheckIsOwner, (LPARAM)&ctx);
	return ctx.IsOwner;
}

bool oWinIsRenderTarget(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	return !!(GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) & oEF_RENDER_TARGET);
}

bool oWinSetIsRenderTarget(HWND _hWnd, bool _IsRenderTarget)
{
	oWIN_CHECK(_hWnd);
	LONG_PTR flags = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
	if (_IsRenderTarget) flags |= oEF_RENDER_TARGET;
	else flags &=~ oEF_RENDER_TARGET;
	SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, flags);
	return true;
}

bool oWinIsFullscreenExclusive(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	return !!(GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) & oEF_FULLSCREEN_EXCLUSIVE);
}

oAPI bool oWinSetIsFullscreenExclusive(HWND _hWnd, bool _IsFullscreenExclusive)
{
	oWIN_CHECK(_hWnd);
	LONG_PTR flags = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
	if (_IsFullscreenExclusive) flags |= oEF_FULLSCREEN_EXCLUSIVE;
	else flags &=~ oEF_FULLSCREEN_EXCLUSIVE;
	SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, flags);
	return true;
}

bool oWinAltF4IsEnabled(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	return !!(GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) & oEF_ALT_F4_ENABLED);
}

bool oWinAltF4Enable(HWND _hWnd, bool _Enabled)
{
	oWIN_CHECK(_hWnd);
	LONG_PTR flags = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
	if (_Enabled) flags |= oEF_ALT_F4_ENABLED;
	else flags &=~ oEF_ALT_F4_ENABLED;
	SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, flags);
	return true;
}

bool oWinExists(HWND _hWnd)
{
	return !!::IsWindow(_hWnd);
}

bool oWinHasFocus(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	return oWinExists(_hWnd) && _hWnd == ::GetForegroundWindow();
}

bool oWinSetFocus(HWND _hWnd, bool _Focus)
{
	// @oooii-kevin: Technically this can be called from other threads as we can give focus to another window
	// @oooii-tony: This shouldn't be true, there's several calls below so thus
	// this isn't atomic. See who complains by reenabling the check here.
	oWIN_CHECK(_hWnd);
	if (_Focus)
	{
		::SetForegroundWindow(_hWnd);
		::SetActiveWindow(_hWnd);
		::SetFocus(_hWnd);
		return true;
	}
	
	else
		return oWinSetFocus(GetNextWindow(_hWnd, GW_HWNDNEXT), true);
}

bool oWinIsEnabled(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	oVB_RETURN(IsWindowEnabled(_hWnd));
	return true;
}

bool oWinEnable(HWND _hWnd, bool _Enabled)
{
	oWIN_CHECK(_hWnd);
	EnableWindow(_hWnd, BOOL(_Enabled));
	return true;
}

bool oWinIsAlwaysOnTop(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	return !!(GetWindowLong(_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
}

bool oWinSetAlwaysOnTop(HWND _hWnd, bool _AlwaysOnTop)
{
	oWIN_CHECK(_hWnd);
	RECT r;
	oVB_RETURN(GetWindowRect(_hWnd, &r));
	return !!::SetWindowPos(_hWnd, _AlwaysOnTop ? HWND_TOPMOST : HWND_TOP, r.left, r.top, oWinRectW(r), oWinRectH(r), IsWindowVisible(_hWnd) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
}

bool oWinGetClientRect(HWND _hWnd, RECT* _pRect)
{
	oWIN_CHECK(_hWnd);

	if (!_pRect)
		return oErrorSetLast(std::errc::invalid_argument);

	// Discount status bar from dimensions
	oVB_RETURN(GetClientRect(_hWnd, _pRect));
	_pRect->bottom = __max(_pRect->top, _pRect->bottom - oWinGetStatusBarHeight(_hWnd));

	// Translate to proper offset
	POINT p = { _pRect->left, _pRect->top };
	HWND hParent = GetParent(_hWnd);
	if (!hParent)
		oVB_RETURN(ClientToScreen(_hWnd, &p));
	else
	{
		SetLastError(0); // differentiate between 0-means-failure, and 0,0 adjustment
		if (!MapWindowPoints(_hWnd, hParent, &p, 1) && GetLastError() != S_OK)
			return oWinSetLastError();
	}
	
	*_pRect = oWinRectTranslate(*_pRect, p);

	return true;
}

RECT oWinGetParentRect(HWND _hWnd, HWND _hExplicitParent)
{
	HWND hParent = _hExplicitParent ? _hExplicitParent : GetParent(_hWnd);
	RECT rParent;
	if (hParent)
		oVERIFY(oWinGetClientRect(hParent, &rParent));
	else
	{
		oDISPLAY_DESC dd;
		oDisplayEnum(oWinGetDisplayIndex(_hWnd), &dd);
		rParent = oWinRectWH(dd.WorkareaPosition, dd.WorkareaSize);
	}
	return rParent;
}

RECT oWinGetRelativeRect(HWND _hWnd, HWND _hExplicitParent)
{
	HWND hParent = _hExplicitParent ? _hExplicitParent : GetParent(_hWnd);
	if (!hParent)
		hParent = GetDesktopWindow();

	RECT r;
	GetWindowRect(_hWnd, &r);
	ScreenToClient(hParent, (POINT*)&r);
	ScreenToClient(hParent, &((POINT*)&r)[1]);
	return r;	
}

oGUI_WINDOW_STATE oWinGetState(HWND _hWnd)
{
	if (!oWinExists(_hWnd)) return oGUI_WINDOW_INVALID;
	
	LONG_PTR style = GetWindowLongPtr(_hWnd, GWL_STYLE);
	if (!(style & WS_VISIBLE)) return oGUI_WINDOW_HIDDEN;

	LONG_PTR extra = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
	if (extra & (oEF_FULLSCREEN_EXCLUSIVE|oEF_FULLSCREEN_COOPERATIVE)) return oGUI_WINDOW_FULLSCREEN;
	
	if (IsIconic(_hWnd)) return oGUI_WINDOW_MINIMIZED;
	if (IsZoomed(_hWnd)) return oGUI_WINDOW_MAXIMIZED;

	return oGUI_WINDOW_RESTORED;
}

static oGUI_WINDOW_STYLE oWinGetStyle(HWND _hWnd)
{
	#define oFIXED_STYLE (WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX)
	#define oSET(_Flag) ((style & (_Flag)) == (_Flag))

	oASSERT(oWinExists(_hWnd) && oWinIsWindowThread(_hWnd), "function must be called on window thread");
	const bool HasMenu = !!GetMenu(_hWnd);
	const bool HasStatusBar = oWinStatusBarShown(_hWnd);

	if (HasStatusBar && oWinIsRenderTarget(_hWnd))
	{
		oTRACE("HWND 0x%x is showing a status bar and is a render target, which is not allowed. The application will now terminate.", _hWnd);
		std::terminate();
	}

	LONG_PTR style = GetWindowLongPtr(_hWnd, GWL_STYLE);
	if (oSET(WS_OVERLAPPEDWINDOW))
	{
		if (HasMenu && HasStatusBar) return oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR;
		else if (HasMenu) return oGUI_WINDOW_SIZABLE_WITH_MENU;
		else if (HasStatusBar) return oGUI_WINDOW_SIZABLE_WITH_STATUSBAR;
		else return oGUI_WINDOW_SIZABLE;
	}
	else if (oSET(oFIXED_STYLE))
	{
		if (HasMenu && HasStatusBar) return oGUI_WINDOW_FIXED_WITH_MENU_AND_STATUSBAR;
		else if (HasMenu) return oGUI_WINDOW_FIXED_WITH_MENU;
		else if (HasStatusBar) return oGUI_WINDOW_FIXED_WITH_STATUSBAR;
		else return oGUI_WINDOW_FIXED;
	}

	LONG_PTR exstyle = GetWindowLongPtr(_hWnd, GWL_EXSTYLE);
	if (exstyle & WS_EX_WINDOWEDGE) return oGUI_WINDOW_DIALOG;
	return oGUI_WINDOW_BORDERLESS;
	
	#undef oFIXED_STYLE
	#undef oSET
}

oGUI_WINDOW_SHAPE_DESC oWinGetShape(HWND _hWnd)
{
	oGUI_WINDOW_SHAPE_DESC s;
	RECT rClient;
	oVERIFY(oWinGetClientRect(_hWnd, &rClient));
	s.ClientSize = oWinRectSize(rClient);
	s.ClientPosition = oWinRectPosition(rClient);
	s.State = oWinGetState(_hWnd);
	s.Style = oWinGetStyle(_hWnd);
	return s;
}

bool oWinSetShape(HWND _hWnd, const oGUI_WINDOW_SHAPE_DESC& _Shape)
{
	oWIN_CHECK(_hWnd);

	oGUI_WINDOW_SHAPE_DESC New = _Shape;
	oGUI_WINDOW_SHAPE_DESC Old = oWinGetShape(_hWnd);
	oASSERT(Old.State != oGUI_WINDOW_INVALID && Old.Style != oGUI_WINDOW_DEFAULT, "");

	if (New.State == oGUI_WINDOW_INVALID)
		New.State = Old.State;

	if (New.Style == oGUI_WINDOW_DEFAULT)
		New.Style = Old.Style;

	if (oGUIStyleHasStatusBar(New.Style) && oWinIsRenderTarget(_hWnd))
		return oErrorSetLast(std::errc::protocol_error, "HWND 0x%x is marked as a render target, disallowing status bar styles to be set", _hWnd);

	if (Old.State == oGUI_WINDOW_RESTORED)
		oWinSaveRestoredPosSize(_hWnd);

	oStd::finally f([&] {SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) &~ oEF_NO_SAVE_RESTORED_POSITION_SIZE);});
	if (New.State == oGUI_WINDOW_FULLSCREEN || New.State == oGUI_WINDOW_MINIMIZED || New.State == oGUI_WINDOW_MAXIMIZED)
		SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) | oEF_NO_SAVE_RESTORED_POSITION_SIZE);

	if (Old.State != New.State)
		SetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STATE, (LONG_PTR)Old.State);

	if (Old.State != oGUI_WINDOW_FULLSCREEN)
		SetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STYLE, (LONG_PTR)Old.Style);

	if (New.State == oGUI_WINDOW_FULLSCREEN)
	{
		New.Style = oGUI_WINDOW_BORDERLESS;
		LONG_PTR extra = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
		if (New.State == oGUI_WINDOW_FULLSCREEN)
			extra |= oEF_FULLSCREEN_COOPERATIVE;
		else
			extra &=~ oEF_FULLSCREEN_COOPERATIVE;
		SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, extra);
	}
	else if (Old.State == oGUI_WINDOW_FULLSCREEN)
	{
		New.Style = (oGUI_WINDOW_STYLE)GetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STYLE);
		SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) & ~(oEF_FULLSCREEN_COOPERATIVE|oEF_FULLSCREEN_EXCLUSIVE));
	}

	if (New.State == oGUI_WINDOW_RESTORED)
	{
		LONG_PTR lParam = GetWindowLongPtr(_hWnd, oGWLP_RESTORED_POSITION);
		const int2 RestoredClientPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		New.ClientPosition = oGUIResolveRectPosition(New.ClientPosition, RestoredClientPosition);
		lParam = GetWindowLongPtr(_hWnd, oGWLP_RESTORED_SIZE);
		const int2 RestoredClientSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		New.ClientSize = oGUIResolveRectPosition(New.ClientSize, RestoredClientSize);
	}

	else if (New.State == oGUI_WINDOW_FULLSCREEN)
	{
		oDISPLAY_DESC dd;
		oVERIFY(oDisplayEnum(oWinGetDisplayIndex(_hWnd), &dd));
		New.ClientPosition = dd.Position;
		New.ClientSize = dd.Mode.Size;
	}

	else
	{
		New.ClientPosition = oGUIResolveRectPosition(New.ClientPosition, Old.ClientPosition);
		New.ClientSize = oGUIResolveRectPosition(New.ClientSize, Old.ClientSize);
	}

	int StatusBarHeight = 0;
	{
		RECT rStatusBar;
		GetClientRect(oWinGetStatusBar(_hWnd), &rStatusBar);
		StatusBarHeight = oWinRectH(rStatusBar);
	}

	DWORD dwAllFlags = (DWORD)GetWindowLongPtr(_hWnd, GWL_STYLE);
	if (Old.Style != New.Style)
	{
		// Change only the bits we mean to change and preserve the others
		const bool HasParent = !!GetParent(_hWnd);
		DWORD dwCurrentStyleFlags = oWinGetStyle(Old.Style, HasParent);
		dwAllFlags &=~ dwCurrentStyleFlags;
		dwAllFlags |= oWinGetStyle(New.Style, HasParent);

		// When in maximized state and doing nothing more than toggling status bar
		// visibility, the client area has changed, so spoof a WM_SIZE to notify
		// the system of that change. If the menu changes, then allow the WM_SIZE to 
		// be sent by oWinShowMenu.
		oVERIFY(oWinShowStatusBar(_hWnd, oGUIStyleHasStatusBar(New.Style)));

		if (Old.State == oGUI_WINDOW_MAXIMIZED && New.State == oGUI_WINDOW_MAXIMIZED 
			&& oGUIStyleHasStatusBar(New.Style) != oGUIStyleHasStatusBar(Old.Style)
			&& oGUIStyleHasMenu(New.Style) == oGUIStyleHasMenu(Old.Style))
		{
			int2 NewMaxSize = Old.ClientSize;
			if (oGUIStyleHasStatusBar(New.Style))
				NewMaxSize.y -= StatusBarHeight;
			else
				NewMaxSize.y += StatusBarHeight;
			SendMessage(_hWnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(NewMaxSize.x, NewMaxSize.y));
		}

		// This will send a temp WM_SIZE event. Another will be sent below, so 
		// squelch this one from making it all the way through to an oGUI_EVENT.
		// However, during a style change to a maximized window, that affects the 
		// menu other calls won't be made, so let this be the authority in that case.
		oWinSetTempChange(_hWnd, !(Old.State == oGUI_WINDOW_MAXIMIZED && New.State == oGUI_WINDOW_MAXIMIZED));
		oWinShowMenu(_hWnd, oGUIStyleHasMenu(New.Style));
		oWinSetTempChange(_hWnd, false);
	}

	// Resolve position and size to a rectangle. Add extra room for the status bar.
	RECT r = oWinRectWH(New.ClientPosition, New.ClientSize);
	if (oGUIStyleHasStatusBar(New.Style))
		r.bottom += StatusBarHeight;

	// @oooii-tony: are these bit-clears needed?
	if (New.State != oGUI_WINDOW_MAXIMIZED)
		dwAllFlags &=~ WS_MAXIMIZE;

	if (New.State != oGUI_WINDOW_MINIMIZED)
		dwAllFlags &=~ WS_MINIMIZE;

	// Transform the rectangle to with-border values for the new style
	oVB_RETURN(AdjustWindowRect(&r, dwAllFlags, !!GetMenu(_hWnd)));

	// Update the flagging of the new style. This won't do anything without a call 
	// to SetWindowPos.
	SetLastError(0); // http://msdn.microsoft.com/en-us/library/ms644898(VS.85).aspx
	oWinSetTempChange(_hWnd, true);
	oVB_RETURN(SetWindowLongPtr(_hWnd, GWL_STYLE, dwAllFlags));
	oWinSetTempChange(_hWnd, false);

	// Allow the system to calculate minimized/maximized sizes for us, so don't
	// modify them here, that will happen in SetWindowPlacement.
	if (!(Old.State == oGUI_WINDOW_FULLSCREEN && New.State == oGUI_WINDOW_MAXIMIZED))
	{
		UINT uFlags = SWP_NOZORDER|SWP_FRAMECHANGED;
		if (New.State == oGUI_WINDOW_MINIMIZED || New.State == oGUI_WINDOW_MAXIMIZED)
			uFlags |= SWP_NOMOVE|SWP_NOSIZE;
		oVB_RETURN(SetWindowPos(_hWnd, 0, r.left, r.top, oWinRectW(r), oWinRectH(r), uFlags));
	}

	// Now handle visibility, min/max/restore
	if (Old.State != New.State)
	{
		WINDOWPLACEMENT WP;
		WP.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(_hWnd, &WP);

		switch (New.State)
		{
			case oGUI_WINDOW_INVALID: case oGUI_WINDOW_HIDDEN:
				WP.showCmd = SW_HIDE;
				break;
			case oGUI_WINDOW_MINIMIZED:
				WP.showCmd = SW_SHOWMINNOACTIVE;
				break;
			case oGUI_WINDOW_MAXIMIZED:
				WP.showCmd = SW_SHOWMAXIMIZED;
				break;
			case oGUI_WINDOW_FULLSCREEN:
			case oGUI_WINDOW_RESTORED:
				WP.showCmd = SW_RESTORE;
				WP.rcNormalPosition = r;
				break;
		}
		oVB_RETURN(SetWindowPlacement(_hWnd, &WP));
	}

	return true;
}

bool oWinRestore(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	HWND hProgMan = FindWindow(0, "Program Manager");
	oASSERT(hProgMan, "Program Manager not found");
	oWinSetFocus(hProgMan);
	oWinSetFocus(_hWnd);
	ShowWindow(_hWnd, SW_SHOWDEFAULT);
	return true;
}

bool oWinAnimate(HWND _hWnd, const RECT& _From, const RECT& _To)
{
	oWIN_CHECK(_hWnd);
	ANIMATIONINFO ai;
	ai.cbSize = sizeof(ai);
	SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);
	if (ai.iMinAnimate)
		return !!DrawAnimatedRects(_hWnd, IDANI_CAPTION, &_From, &_To);
	return true;
}

int oWinGetDisplayIndex(HWND _hWnd)
{
	DISPLAY_DEVICE dev;
	return oWinGetDisplayDevice(MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST), &dev);
}

HBRUSH oWinGetBackgroundBrush(HWND _hWnd)
{
	return (HBRUSH)GetClassLongPtr(_hWnd, GCLP_HBRBACKGROUND);
}

HFONT oWinGetFont(HWND _hWnd)
{
	return (HFONT)SendMessage(_hWnd, WM_GETFONT, 0, 0);
}

HFONT oWinGetDefaultFont()
{
	return (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

bool oWinSetFont(HWND _hWnd, HFONT _hFont)
{
	oWIN_CHECK(_hWnd);
	if (!_hFont)
		_hFont = oWinGetDefaultFont();
	SendMessage(_hWnd, WM_SETFONT, (WPARAM)_hFont, TRUE);
	return true;
}

bool oWinSetText(HWND _hWnd, const char* _Text, int _SubItemIndex)
{
	oWIN_CHECK(_hWnd);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case oGUI_CONTROL_COMBOTEXTBOX:
			if (_SubItemIndex == oInvalid && !SetWindowText(_hWnd, _Text))
				return oWinSetLastError();
			// pass through
		case oGUI_CONTROL_COMBOBOX:
			if (_SubItemIndex != oInvalid)
			{
				oVERIFY(oWinControlDeleteSubItem(_hWnd, _Text, _SubItemIndex));
				oVERIFY(oWinControlInsertSubItem(_hWnd, _Text, _SubItemIndex));
			}
			break;

		case oGUI_CONTROL_TAB:
		{
			TCITEM tci;
			memset(&tci, 0, sizeof(tci));
			tci.dwStateMask = TCIF_TEXT;
			tci.pszText = (LPSTR)_Text;

			if (!TabCtrl_SetItem(_hWnd, _SubItemIndex, &tci))
				return oWinSetLastError();

			break;
		}

		case oGUI_CONTROL_FLOATBOX:
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			return oErrorSetLast(std::errc::invalid_argument, "For FloatBoxes and FloatBoxSpinners, use oWinControlSetValue instead.");

		default: 
			if (!SetWindowText(_hWnd, _Text))
				return oWinSetLastError();
	}

	return true;
}

size_t oWinGetTruncatedLength(HWND _hWnd, const char* _StrSource)
{
	oStd::xxlstring temp(_StrSource);

	RECT rClient;
	GetClientRect(_hWnd, &rClient);
	oGDIScopedGetDC hDC(_hWnd);

	if (!oWinShlwapi::Singleton()->PathCompactPathA(hDC, temp, oWinRectW(rClient) + 70)) // @oooii-tony: This constant was measured empirically. I think this should work without the +70, but truncation happens too aggressively.
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer must be at least MAX_PATH (%u) chars big", MAX_PATH);
		return 0;
	}

	if (!oStrcmp(temp, _StrSource))
	{
		oErrorSetLast(std::errc::operation_in_progress, "Truncation not required");
		return 0;
	}

	return temp.length();
}

char* oWinTruncateLeft(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _StrSource)
{
	size_t TruncatedLength = oWinGetTruncatedLength(_hWnd, _StrSource);
	if (TruncatedLength)
	{
		oStd::uri_string Truncated;
		const char* pCopy = _StrSource + oStrlen(_StrSource) - TruncatedLength + 3;
		oASSERT(pCopy >= _StrSource, "");
		if (-1 == oPrintf(_StrDestination, _SizeofStrDestination, "...%s", pCopy))
		{
			oErrorSetLast(std::errc::no_buffer_space, "");
			return nullptr;
		}
	}

	return oErrorGetLast() == std::errc::operation_in_progress ? nullptr : _StrDestination;
}

char* oWinTruncatePath(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _Path)
{
	if (_SizeofStrDestination < MAX_PATH)
		return nullptr; // can't pass this to the function

	oStrcpy(_StrDestination, _SizeofStrDestination, _Path);
	RECT rClient;
	GetClientRect(_hWnd, &rClient);
	oGDIScopedGetDC hDC(_hWnd);

	if (!oWinShlwapi::Singleton()->PathCompactPathA(hDC, _StrDestination, oWinRectW(rClient) + 70)) // @oooii-tony: This constant was measured empirically. I think this should work without the +70, but truncation happens too aggressively.
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer must be at least MAX_PATH (%u) chars big", MAX_PATH);
		return nullptr;
	}

	return _StrDestination;
}

char* oWinGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, int _SubItemIndex)
{
	oWINVP(_hWnd);
	oGUI_CONTROL_TYPE type = oGUI_CONTROL_UNKNOWN;
	if (_SubItemIndex != oInvalid)
		type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			size_t len = (size_t)ComboBox_GetLBTextLen(_hWnd, _SubItemIndex);
			if (len > _SizeofStrDestination)
			{
				oErrorSetLast(std::errc::no_buffer_space);
				return nullptr;
			}
			else
			{
				if (CB_ERR != ComboBox_GetLBText(_hWnd, _SubItemIndex, _StrDestination))
					return _StrDestination;
			}
		}

		case oGUI_CONTROL_TAB:
		{
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)_StrDestination;
			item.cchTextMax = oInt(_SizeofStrDestination);
			if (TabCtrl_GetItem(_hWnd, _SubItemIndex, &item))
				return _StrDestination;
			else
			{
				oWinSetLastError();
				return nullptr;
			}
		}

		default:
			if (GetWindowText(_hWnd, _StrDestination, oInt(_SizeofStrDestination)))
				return _StrDestination;
			else
			{
				oWinSetLastError();
				return nullptr;
			}
	}
}

HICON oWinGetIcon(HWND _hWnd, bool _BigIcon)
{
	oWINVP(_hWnd);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case oGUI_CONTROL_ICON: return (HICON)SendMessage(_hWnd, STM_GETICON, 0, 0);
		default: return (HICON)SendMessage(_hWnd, WM_GETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), 0);
	}
}

bool oWinSetIconAsync(HWND _hWnd, HICON _hIcon, bool _BigIcon)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hWnd);
	if (oWinIsWindowThread(_hWnd))
	{
		switch (type)
		{
			case oGUI_CONTROL_ICON: SendMessage(_hWnd, STM_SETICON, (WPARAM)_hIcon, 0); break;
			default: SendMessage(_hWnd, WM_SETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), (LPARAM)_hIcon); break;
		}

		return true;
	}
	
	else
	{
		switch (type)
		{
			case oGUI_CONTROL_ICON: PostMessage(_hWnd, STM_SETICON, (WPARAM)_hIcon, 0); break;
			default: PostMessage(_hWnd, WM_SETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), (LPARAM)_hIcon); break;
		}

		return oErrorSetLast(std::errc::operation_not_permitted);
	}
}

bool oWinSetIcon(HWND _hWnd, HICON _hIcon, bool _BigIcon)
{
	oWINVP(_hWnd);
	return oWinSetIconAsync(_hWnd, _hIcon, _BigIcon);
}

static const char* oSubclassFloatBoxFormat = "%.04f";
static UINT_PTR oSubclassFloatBoxID = 0x13370000;
static LRESULT CALLBACK oSubclassProcFloatBox(HWND _hControl, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, UINT_PTR _uIdSubclass, DWORD_PTR _dwRefData)
{
	if (_uIdSubclass == oSubclassFloatBoxID)
	{
		switch (_uMsg)
		{
			case WM_KILLFOCUS:
			{
				oStd::mstring text;
				GetWindowText(_hControl, text, (int)text.capacity());
				float f = 0.0f;
				oStd::atof(text, &f);
				oPrintf(text, oSubclassFloatBoxFormat, f);
				SetWindowText(_hControl, text);
				break;
			}

			case WM_SETTEXT:
			{
				float f = 0.0f;
				oStd::mstring text = (const wchar_t*)_lParam; // handle wchar
				if (!oStd::atof(text, &f))
					return FALSE;
				
				// Ensure consistent formatting
				oPrintf(text, oSubclassFloatBoxFormat, f);
				oStd::mwstring wtext = text;
				oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, (LPARAM)wtext.c_str());
				return FALSE;
			}

			case WM_GETDLGCODE:
			{
				// probably copy/paste, so let that through
				if ((GetKeyState(VK_LCONTROL) & 0x1000) || (GetKeyState(VK_RCONTROL) & 0x1000))
					return DLGC_WANTALLKEYS;

				oStd::mstring text;
				GetWindowText(_hControl, text, (int)text.capacity());

				// refresh/commit if the user presses enter
				if (_wParam == VK_RETURN)
				{
					SetWindowText(_hControl, text.c_str());
					SendMessage(_hControl, EM_SETSEL, text.length(), text.length());
					return DLGC_WANTARROWS;
				}

				// Only consider allowable keys at all
				static const WPARAM sAllowableKeys[] = 
				{
					'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '.', '-', '+',
					VK_BACK, VK_DELETE, VK_HOME, VK_END,
				};
				int i = 0;
				for (; i < oCOUNTOF(sAllowableKeys); i++)
					if (sAllowableKeys[i] == _wParam) break;
				
				if (i >= oCOUNTOF(sAllowableKeys))
					return DLGC_WANTARROWS;

				DWORD dwStart = oInvalid, dwEnd = oInvalid;
				SendMessage(_hControl, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

				// allow only one '.', unless it's in the selecte d text about to be replaced
				bool selHasDot = false;
				for (DWORD i = dwStart; i < dwEnd; i++)
					if (text[i] == '.')
					{
						selHasDot = true;
						break;
					}

				if (!selHasDot && _wParam == '.' && strchr(text, '.'))
					return DLGC_WANTARROWS;

				// only one of either + or -, and it better be at the front
				if ((_wParam == '+' || _wParam == '-') && (dwStart != 0 || strchr(text, '+') || strchr(text, '-')))
					return DLGC_WANTARROWS;

				return DLGC_WANTALLKEYS;
			}

			default:
				break;
		}
	}

	return oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
}

static UINT_PTR oSubclassTabID = 0x13370001;
static LRESULT CALLBACK oSubclassProcTab(HWND _hControl, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, UINT_PTR _uIdSubclass, DWORD_PTR _dwRefData)
{
	if (_uIdSubclass == oSubclassTabID)
	{
		switch (_uMsg)
		{
			case WM_CTLCOLORSTATIC:
				return (INT_PTR)0;
			case WM_COMMAND:
			case WM_NOTIFY:
				return SendMessage(GetParent(_hControl), _uMsg, _wParam, _lParam);
			default:
				break;
		}
	}

	return oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
}

static UINT_PTR oSubclassGroupID = 0x13370002;
static LRESULT CALLBACK oSubclassProcGroup(HWND _hControl, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, UINT_PTR _uIdSubclass, DWORD_PTR _dwRefData)
{
	if (_uIdSubclass == oSubclassGroupID)
	{
		switch (_uMsg)
		{
			case WM_CTLCOLORSTATIC:
			{
				HBRUSH hBrush = oWinGetBackgroundBrush(_hControl);
				return (INT_PTR)hBrush;
			}
			case WM_COMMAND:
			case WM_NOTIFY:
				return SendMessage(GetParent(_hControl), _uMsg, _wParam, _lParam);

			default:
				break;
		}
	}

	return oWinCommCtrl::Singleton()->DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
}

static bool OnCreateAddSubItems(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	if (strchr(_Desc.Text, '|'))
	{
		oVERIFY_R(oWinControlAddSubItems(_hControl, _Desc.Text));
		oVERIFY_R(oWinControlSelectSubItem(_hControl, 0));
	}

	return true;
}

static bool OnCreateTab(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	oVERIFY_R(oWinCommCtrl::Singleton()->SetWindowSubclass(_hControl, oSubclassProcTab, oSubclassTabID, (DWORD_PTR)nullptr));
	return OnCreateAddSubItems(_hControl, _Desc);
}

static bool OnCreateGroup(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	return !!oWinCommCtrl::Singleton()->SetWindowSubclass(_hControl, oSubclassProcGroup, oSubclassGroupID, (DWORD_PTR)nullptr);
}

static bool OnCreateFloatBox(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	oVERIFY_R(oWinCommCtrl::Singleton()->SetWindowSubclass(_hControl, oSubclassProcFloatBox, oSubclassFloatBoxID, (DWORD_PTR)nullptr));
	SetWindowText(_hControl, _Desc.Text);
	return true;
}

static bool OnCreateIcon(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	oWinControlSetIcon(_hControl, (HICON)_Desc.Text);
	return true;
}

static bool OnCreateFloatBoxSpinner(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	oGUI_CONTROL_DESC ActualFloatBoxDesc = _Desc;
	ActualFloatBoxDesc.Type = oGUI_CONTROL_FLOATBOX;
	HWND hActualFloatBox = oWinControlCreate(ActualFloatBoxDesc);
	if (hActualFloatBox)
	{
		SendMessage(_hControl, UDM_SETBUDDY, (WPARAM)hActualFloatBox, 0);
		SendMessage(_hControl, UDM_SETRANGE32, (WPARAM)std::numeric_limits<int>::lowest(), (LPARAM)std::numeric_limits<int>::max());
	}

	else
		return oWinSetLastError();

	return true;
}

static bool OnCreateMarquee(HWND _hControl, const oGUI_CONTROL_DESC& _Desc)
{
	SendMessage(_hControl, PBM_SETMARQUEE, 1, 0);
	return true;
}

static int2 GetSizeExplicit(const oGUI_CONTROL_DESC& _Desc)
{
	return _Desc.Size;
}

static int2 GetSizeButtonDefault(const oGUI_CONTROL_DESC& _Desc)
{
	return oGUIResolveRectSize(_Desc.Size, int2(75, 23));
}

static int2 GetSizeIcon(const oGUI_CONTROL_DESC& _Desc)
{
	int2 iconSize = oGDIGetIconSize((HICON)_Desc.Text);
	return oGUIResolveRectSize(_Desc.Size, iconSize);
}

struct CONTROL_CREATION_DESC
{
	const char* ClassName;
	DWORD dwStyle;
	DWORD dwExStyle;
	bool SetText;
	bool (*FinishCreation)(HWND _hControl, const oGUI_CONTROL_DESC& _Desc);
	int2 (*GetSize)(const oGUI_CONTROL_DESC& _Desc);
};

static const CONTROL_CREATION_DESC& oWinControlGetCreationDesc(oGUI_CONTROL_TYPE _Type)
{
	// === IF ADDING A NEW TYPE, MAKE SURE YOU ADD AN ENTRY TO oWinControlGetType ===

	static const CONTROL_CREATION_DESC sDescs[] = 
	{
		{ "Unknown", 0, 0, false, nullptr, GetSizeExplicit, },
		{ "Button", WS_VISIBLE|WS_CHILD|BS_GROUPBOX, 0, true, OnCreateGroup, GetSizeExplicit, },
		{ "Button", WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_PUSHBUTTON, 0, true, nullptr, GetSizeButtonDefault, },
		{ "Button", WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_TOP|BS_AUTOCHECKBOX, 0, true, nullptr, GetSizeExplicit, },
		{ "Button", WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_TOP|BS_AUTORADIOBUTTON, 0, true, nullptr, GetSizeExplicit, },
		{ "Static", WS_VISIBLE|WS_CHILD|SS_EDITCONTROL, 0, true, nullptr, GetSizeExplicit, },
		{ "SysLink", WS_VISIBLE|WS_CHILD|WS_TABSTOP|LWS_NOPREFIX, 0, true, nullptr, GetSizeExplicit, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_READONLY|ES_NOHIDESEL|ES_AUTOHSCROLL, 0, true, nullptr, GetSizeExplicit, },
		{ "Static", WS_VISIBLE|WS_CHILD|SS_ICON|SS_REALSIZECONTROL, 0, false, OnCreateIcon, GetSizeIcon, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN, WS_EX_CLIENTEDGE, 0, nullptr, GetSizeExplicit, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN, WS_EX_CLIENTEDGE, 0, nullptr, GetSizeExplicit, },
		{ "Edit", WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_RIGHT, WS_EX_CLIENTEDGE, false, OnCreateFloatBox, GetSizeExplicit, },
		{ UPDOWN_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|UDS_ALIGNRIGHT|UDS_ARROWKEYS|UDS_HOTTRACK|UDS_NOTHOUSANDS|UDS_WRAP, 0,false, OnCreateFloatBoxSpinner, GetSizeExplicit, },
		{ "Combobox", WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_HASSTRINGS, 0, false, OnCreateAddSubItems, GetSizeExplicit, },
		{ "Combobox", WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWN|CBS_HASSTRINGS, 0, false, OnCreateAddSubItems, GetSizeExplicit, },
		{ PROGRESS_CLASS, WS_VISIBLE|WS_CHILD, 0, true, nullptr, GetSizeExplicit, },
		{ PROGRESS_CLASS, WS_VISIBLE|WS_CHILD|PBS_MARQUEE, 0, true, OnCreateMarquee, GetSizeExplicit, },
		{ WC_TABCONTROL, WS_VISIBLE|WS_CHILD|WS_TABSTOP, WS_EX_CONTROLPARENT, true, OnCreateTab, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|TBS_NOTICKS, 0, true, nullptr, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|TBS_NOTICKS|TBS_ENABLESELRANGE, 0, true, nullptr, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP, 0, true, nullptr, GetSizeExplicit, }, 
	};
	static_assert(oGUI_CONTROL_TYPE_COUNT == oCOUNTOF(sDescs), "oGUI_CONTROL_TYPE_COUNT count mismatch");
	return sDescs[_Type];
}

HWND oWinControlCreate(const oGUI_CONTROL_DESC& _Desc)
{
	if (!_Desc.hParent || _Desc.Type == oGUI_CONTROL_UNKNOWN)
		return (HWND)oErrorSetLast(std::errc::invalid_argument);
	oWINVP((HWND)_Desc.hParent);

	const CONTROL_CREATION_DESC& CCDesc = oWinControlGetCreationDesc(_Desc.Type);
	int2 size = CCDesc.GetSize(_Desc);

	HWND hWnd = CreateWindowEx(
		CCDesc.dwExStyle
		, CCDesc.ClassName
		, (CCDesc.SetText && _Desc.Text) ? _Desc.Text : ""
		, CCDesc.dwStyle | (_Desc.StartsNewGroup ? WS_GROUP : 0)
		, _Desc.Position.x
		, _Desc.Position.y
		, size.x
		, size.y
		, (HWND)_Desc.hParent
		, (HMENU)_Desc.ID
		, nullptr
		, nullptr);

	if (hWnd)
	{
		oWinControlSetFont(hWnd, (HFONT)_Desc.hFont);
		if (CCDesc.FinishCreation)
		{
			if (!CCDesc.FinishCreation(hWnd, _Desc))
			{
				DestroyWindow(hWnd);
				hWnd = nullptr;
			}
		}
	}

	else
		oWinSetLastError();

	return hWnd;
}

bool oWinControlDefaultOnNotify(HWND _hControl, const NMHDR& _NotifyMessageHeader, LRESULT* _plResult, oGUI_CONTROL_TYPE _Type)
{
	oWIN_CHECK(_hControl);
	bool ShortCircuit = false;
	if (_Type == oGUI_CONTROL_UNKNOWN)
		_Type = oWinControlGetType(_NotifyMessageHeader.hwndFrom);
	switch (_Type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			if (_NotifyMessageHeader.code == UDN_DELTAPOS)
			{
				HWND hBuddyFloatBox =oWinControlGetBuddy(_NotifyMessageHeader.hwndFrom);
				if (hBuddyFloatBox)
				{
					const NMUPDOWN& ud = (const NMUPDOWN&)_NotifyMessageHeader;
					float f = oWinControlGetFloat(hBuddyFloatBox);
					oWinControlSetValue(hBuddyFloatBox, f + 0.01f * ud.iDelta);
				}
			}
			ShortCircuit = true;
			break;

		case oGUI_CONTROL_HYPERLABEL:
		{
			if (_NotifyMessageHeader.code == NM_CLICK || _NotifyMessageHeader.code == NM_RETURN)
			{
				const NMLINK& NMLink = (const NMLINK&)_NotifyMessageHeader;
				oStd::xlstring asMultiByte(NMLink.item.szUrl);
				oVERIFY(oWinSystemOpenDocument(asMultiByte));
			}

			ShortCircuit = true;
			break;
		}

		default: break;
	}

	if (_plResult)
		*_plResult = FALSE;

	return ShortCircuit;
}

bool oWinControlToAction(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, oGUI_ACTION_DESC* _pAction, LRESULT* _pLResult)
{
	*_pLResult = 0;
	bool Handled = true;

	switch (_uMsg)
	{
		case WM_COMMAND:
		{
			_pAction->DeviceType = oGUI_INPUT_DEVICE_CONTROL;
			_pAction->DeviceID = LOWORD(_wParam);
			_pAction->hWindow = (oGUI_WINDOW)_hWnd;
			_pAction->Key = oGUI_KEY_NONE;
			_pAction->Position = 0.0f;

			if (!_lParam)
			{
				_pAction->Action = (HIWORD(_wParam) == 1) ? oGUI_ACTION_HOTKEY : oGUI_ACTION_MENU;
				_pAction->ActionCode = oInvalid;
			}
			else
			{
				_pAction->hWindow = (oGUI_WINDOW)_lParam;
				_pAction->Action = oGUI_ACTION_CONTROL_ACTIVATED;
				_pAction->ActionCode = HIWORD(_wParam);
			}

			break;
		}

		case WM_HSCROLL:
		{
			if (_lParam != 0)
			{
				_pAction->hWindow = (oGUI_WINDOW)(_lParam);
				_pAction->DeviceType = oGUI_INPUT_DEVICE_CONTROL;
				_pAction->DeviceID = oInvalid;
				_pAction->Key = oGUI_KEY_NONE;
				_pAction->Position = 0.0f;
				_pAction->ActionCode = oInt(_wParam);
				switch (LOWORD(_wParam))
				{
					case TB_ENDTRACK: _pAction->Action = oGUI_ACTION_CONTROL_DEACTIVATED; break;
					default: _pAction->Action = oGUI_ACTION_CONTROL_ACTIVATED; break;
				}
				break;
			}

			Handled = false;
			break;
		}

		case WM_NOTIFY:
		{
			LRESULT lResult = FALSE;
			const NMHDR& nmhdr = *(const NMHDR*)_lParam;
			*_pAction = oGUI_ACTION_DESC();
			_pAction->DeviceType = oGUI_INPUT_DEVICE_CONTROL;
			_pAction->DeviceID = oInt(nmhdr.idFrom);
			_pAction->hWindow = (oGUI_WINDOW)nmhdr.hwndFrom;
			_pAction->Position = 0.0f;
			_pAction->ActionCode = nmhdr.code;

			oGUI_CONTROL_TYPE type = oWinControlGetType(nmhdr.hwndFrom);
			switch (type)
			{
				case oGUI_CONTROL_TAB:
				{
					switch (_pAction->ActionCode)
					{
						case TCN_SELCHANGING: _pAction->Action = oGUI_ACTION_CONTROL_SELECTION_CHANGING; break;
						case TCN_SELCHANGE: _pAction->Action = oGUI_ACTION_CONTROL_SELECTION_CHANGED; break;
						default: break;
					}

					break;
				}

				case oGUI_CONTROL_BUTTON:
				{
					switch (_pAction->ActionCode)
					{
						case BN_CLICKED: _pAction->Action = oGUI_ACTION_CONTROL_ACTIVATED; break;
						default: break;
					}

					break;
				}

				default: break;
			}

			Handled = !oWinControlDefaultOnNotify(_hWnd, nmhdr, _pLResult, type);
			break;
		}

		default: Handled = false; break;
	}

	return Handled;
}

bool oWinControlIsVisible(HWND _hControl)
{
	return !!(GetWindowLongPtr(_hControl, GWL_STYLE) & WS_VISIBLE);
}

bool oWinControlSetVisible(HWND _hControl, bool _Visible)
{
	if (!ShowWindow(_hControl, _Visible ? SW_SHOWNA : SW_HIDE))
		oVB_RETURN(RedrawWindow(_hControl, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW));
	return true;
}

int oWinControlGetNumSubItems(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX: return ComboBox_GetCount(_hControl);
		case oGUI_CONTROL_TAB: return TabCtrl_GetItemCount(_hControl);
		default: return 0;
	}
}

bool oWinControlClearSubItems(HWND _hControl)
{
	oWIN_CHECK(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
			ComboBox_ResetContent(_hControl);
			return true;
		case oGUI_CONTROL_TAB:
			if (!TabCtrl_DeleteAllItems(_hControl))
				return oWinSetLastError();
			return true;
		default:
				break;
	}

	return true;
}

int oWinControlInsertSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex)
{
	oWIN_CHECK(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			int index = CB_ERR;
			if (_SubItemIndex >= 0 && _SubItemIndex < ComboBox_GetCount(_hControl))
				index = ComboBox_InsertString(_hControl, _SubItemIndex, _SubItemText);
			else
				index = ComboBox_AddString(_hControl, _SubItemText);

			if (index == CB_ERRSPACE)
			{
				oErrorSetLast(std::errc::no_buffer_space, "String is too large");
				index = CB_ERR;
			}

			return index;
		}

		case oGUI_CONTROL_TAB:
		{
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)_SubItemText;
			int count = TabCtrl_GetItemCount(_hControl);
			int index = TabCtrl_InsertItem(_hControl, ((_SubItemIndex >= 0) ? _SubItemIndex : count), &item);
			if (CB_ERR == index)
				oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);
			return index;
		}

		default:
			break;
	}

	oErrorSetLastBadType(_hControl, type);
	return oInvalid;
}

bool oWinControlDeleteSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex)
{
	oWIN_CHECK(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			if (_SubItemIndex >= 0)
				ComboBox_DeleteString(_hControl, _SubItemIndex);
			break;
		}

		case oGUI_CONTROL_TAB:
			TabCtrl_DeleteItem(_hControl, _SubItemIndex);
			break;

			default:
				break;
		}

	return true;
}

bool oWinControlAddSubItems(HWND _hControl, const char* _DelimitedString, char _Delimiter)
{
	oWIN_CHECK(_hControl);
	char delim[2] = { _Delimiter, 0 };
	char* ctx = nullptr;
	const char* tok = oStrTok(_DelimitedString, delim, &ctx);
	while (tok)
	{
		oWinControlInsertSubItem(_hControl, tok, oInvalid);
		tok = oStrTok(nullptr, delim, &ctx);
	}

	if (!oStrTokFinishedSuccessfully(&ctx))
		return oErrorSetLast(std::errc::protocol_error, "Failed to parse tokenized string for combobox values");
	return true;
}

int oWinControlFindSubItem(HWND _hControl, const char* _SubItemText)
{
	int index = CB_ERR;
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
		{
			int index = ComboBox_FindStringExact(_hControl, 0, _SubItemText);
			if (index == CB_ERR)
				oErrorSetLast(std::errc::invalid_argument, "Text %s was not found in %s %p (%d)", oSAFESTRN(_SubItemText), oStd::as_string(type), _hControl, GetDlgCtrlID(_hControl));
			break;
		}

		case oGUI_CONTROL_TAB:
		{
			oStd::mstring text;
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)text.c_str();
			item.cchTextMax = static_cast<int>(text.capacity());
			const int kCount = TabCtrl_GetItemCount(_hControl);
			for (int i = 0; i < kCount; i++)
			{
				if (TabCtrl_GetItem(_hControl, i, &item))
				{
					if (!oStrcmp(text, _SubItemText))
					{
						index = i;
						break;
					}
				}
			}

			break;
		}

	default:
		oErrorSetLastBadType(_hControl, type);
		break;
	}

	return index;
}

bool oWinControlSelectSubItem(HWND _hControl, int _SubItemIndex)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
			if (CB_ERR == ComboBox_SetCurSel(_hControl, _SubItemIndex))
				return oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);
			break;

		case oGUI_CONTROL_TAB:
		{
			HWND hParent = GetParent(_hControl);
			int index = TabCtrl_GetCurSel(_hControl);
			NMHDR nm;
			nm.hwndFrom = _hControl;
			nm.idFrom = GetDlgCtrlID(_hControl);
			if (-1 != index)
			{
				nm.code = TCN_SELCHANGING;
				SendMessage(hParent, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
			}

			index = TabCtrl_SetCurSel(_hControl, _SubItemIndex);
			if (-1 == index)
				return oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);

			nm.code = TCN_SELCHANGE;
			SendMessage(hParent, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);

			break;
		}
		default:
			oErrorSetLastBadType(_hControl, type);
	}

	return true;
}

bool oWinControlSelectSubItemRelative(HWND _hControl, int _Offset)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	int next = -1, count = 0;
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX:
			count = ComboBox_GetCount(_hControl);
			next = ComboBox_GetCurSel(_hControl) + _Offset;
			while (next < 0) next += count;
			while (next >= count) next -= count;
			if (CB_ERR == ComboBox_SetCurSel(_hControl, next))
				return oErrorSetLastBadIndex(_hControl, type, next);
			break;

		case oGUI_CONTROL_TAB:
			count = TabCtrl_GetItemCount(_hControl);
			next = TabCtrl_GetCurSel(_hControl) + _Offset;
			while (next < 0) next += count;
			while (next >= count) next -= count;
			if (-1 == TabCtrl_SetCurSel(_hControl, next))
				return oErrorSetLastBadIndex(_hControl, type, next);
			break;

		default:
			oErrorSetLastBadType(_hControl, type);
	}

	return true;
}

int oWinControlGetSelectedSubItem(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_COMBOBOX:
		case oGUI_CONTROL_COMBOTEXTBOX: return ComboBox_GetCurSel(_hControl);
		case oGUI_CONTROL_TAB: return TabCtrl_GetCurSel(_hControl);
		default: break;
	}
	return oInvalid;
}

oGUI_CONTROL_TYPE oWinControlGetType(HWND _hControl)
{
	oStd::mstring ClassName;
	if (!GetClassName(_hControl, ClassName, static_cast<int>(ClassName.capacity())))
		return oGUI_CONTROL_UNKNOWN;
	
	if (!oStricmp("Button", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hControl, GWL_STYLE);
		switch (dwStyle)
		{
			case BS_GROUPBOX: return oGUI_CONTROL_GROUPBOX;
			case BS_PUSHBUTTON: 
			case BS_DEFPUSHBUTTON: return oGUI_CONTROL_BUTTON;
			case BS_AUTOCHECKBOX: return oGUI_CONTROL_CHECKBOX;
			case BS_AUTORADIOBUTTON: return oGUI_CONTROL_RADIOBUTTON;
			default: return oGUI_CONTROL_UNKNOWN;
		}
	}

	else if (!oStricmp("Static", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle & SS_ICON)
			return oGUI_CONTROL_ICON;
		else if (dwStyle & SS_SIMPLE)
			return oGUI_CONTROL_LABEL;
		else
			return oGUI_CONTROL_UNKNOWN;
	}

	else if (!oStricmp("Edit", ClassName))
	{
		DWORD dwStyle = 0xffff & (DWORD)GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle == (dwStyle & oWinControlGetCreationDesc(oGUI_CONTROL_LABEL_SELECTABLE).dwStyle))
			return oGUI_CONTROL_LABEL_SELECTABLE;

			DWORD_PTR data;
		if (oWinCommCtrl::Singleton()->GetWindowSubclass(_hControl, oSubclassProcFloatBox, oSubclassFloatBoxID, &data))
			return oGUI_CONTROL_FLOATBOX;

		else if (dwStyle == (dwStyle & oWinControlGetCreationDesc(oGUI_CONTROL_TEXTBOX).dwStyle))
			return oGUI_CONTROL_TEXTBOX;

		else
			return oGUI_CONTROL_UNKNOWN;
	}

	else if (!oStricmp("ComboBox", ClassName))
	{
		LONG dwStyle = 0xf & GetWindowLong(_hControl, GWL_STYLE);
		switch (dwStyle)
		{
			case CBS_DROPDOWNLIST: return oGUI_CONTROL_COMBOBOX;
			case CBS_DROPDOWN: return oGUI_CONTROL_COMBOTEXTBOX;
			default: return oGUI_CONTROL_UNKNOWN;
		}
	}

	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_FLOATBOX_SPINNER).ClassName, ClassName))
		return oGUI_CONTROL_FLOATBOX_SPINNER;

	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_HYPERLABEL).ClassName, ClassName))
		return oGUI_CONTROL_HYPERLABEL;

	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_TAB).ClassName, ClassName))
	{
		// Don't identify a generic tab as an oGUI tab, only ones that
		// behave properly
		DWORD_PTR data;
		if (oWinCommCtrl::Singleton()->GetWindowSubclass(_hControl, oSubclassProcTab, oSubclassTabID, &data))
			return oGUI_CONTROL_TAB;
		return oGUI_CONTROL_UNKNOWN;
	}
	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_PROGRESSBAR).ClassName, ClassName))
	{
		DWORD dwStyle = (DWORD)GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle & PBS_MARQUEE)
			return oGUI_CONTROL_PROGRESSBAR_UNKNOWN;
		return oGUI_CONTROL_PROGRESSBAR;
	}
	else if (!oStricmp(oWinControlGetCreationDesc(oGUI_CONTROL_SLIDER).ClassName, ClassName))
	{
		DWORD dwStyle = (DWORD)GetWindowLong(_hControl, GWL_STYLE);

		if (dwStyle & TBS_ENABLESELRANGE)
			return oGUI_CONTROL_SLIDER_SELECTABLE;
		else if (dwStyle & TBS_NOTICKS)
			return oGUI_CONTROL_SLIDER;
		else
			return oGUI_CONTROL_SLIDER_WITH_TICKS;
	}
	return oGUI_CONTROL_UNKNOWN;
}

int2 oWinControlGetInitialSize(oGUI_CONTROL_TYPE _Type, const int2& _Size)
{
	oGUI_CONTROL_DESC d;
	d.Type = _Type;
	d.Size = _Size;
	return oWinControlGetCreationDesc(_Type).GetSize(d);
}

bool oWinControlIsTabStop(HWND _hControl)
{
	oWIN_CHECK(_hControl);
	return IsWindowEnabled(_hControl) && IsWindowVisible(_hControl) && ((GetWindowLong(_hControl, GWL_STYLE) & WS_TABSTOP) == WS_TABSTOP);
}

int oWinControlGetID(HWND _hControl)
{
	return GetDlgCtrlID(_hControl);
}

HWND oWinControlGetFromID(HWND _hParent, unsigned short _ID)
{
	return GetDlgItem(_hParent, _ID);
}

RECT oWinControlGetRect(HWND _hControl)
{
	RECT r;
	GetClientRect(_hControl, &r);
	HWND hParent = GetParent(_hControl);
	if (hParent)
		MapWindowPoints(_hControl, hParent, (LPPOINT)&r, 2);
	return r;
}

char* oWinControlGetSelectedText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hControl)
{
	oWINVP(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
	case oGUI_CONTROL_FLOATBOX_SPINNER:
		_hControl = oWinControlGetBuddy(_hControl);
		// pass through
	case oGUI_CONTROL_TEXTBOX:
	case oGUI_CONTROL_TEXTBOX_SCROLLABLE:
	case oGUI_CONTROL_COMBOTEXTBOX:
	case oGUI_CONTROL_FLOATBOX:
	{
			uint start = 0, end = 0;
			SendMessage(_hControl, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
			size_t len = end - start;
	if (len >= _SizeofStrDestination)
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer too small to receive string");
		return nullptr;
	}

			if (!GetWindowText(_hControl, _StrDestination, oInt(_SizeofStrDestination)))
	{
		oWinSetLastError();
		return nullptr;
	}

			if (start != end)
	{
		if (start)
			oStrcpy(_StrDestination, _SizeofStrDestination, _StrDestination + start);
		_StrDestination[len] = 0;
	}

	return _StrDestination;
		}

		default:
		oErrorSetLastBadType(_hControl, type);
		return nullptr;
	}
}

bool oWinControlSelect(HWND _hControl, int _Start, int _Length)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass thru
		case oGUI_CONTROL_FLOATBOX:
		case oGUI_CONTROL_TEXTBOX:
		case oGUI_CONTROL_TEXTBOX_SCROLLABLE:
		case oGUI_CONTROL_COMBOTEXTBOX:
		case oGUI_CONTROL_LABEL_SELECTABLE:
			Edit_SetSel(_hControl, _Start, _Start+_Length);
			return true;
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			if (_Length)
			{
				SendMessage(_hControl, TBM_SETSELSTART, FALSE, _Start);
				SendMessage(_hControl, TBM_SETSELEND, TRUE, _Start+_Length);
			}
			else
				SendMessage(_hControl, TBM_CLEARSEL, TRUE, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetValue(HWND _hControl, float _Value)
{
	oWIN_CHECK(_hControl);
	oStd::mstring text;
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass through
		default:
			oPrintf(text, oSubclassFloatBoxFormat, _Value);
			oVB(SetWindowText(_hControl, text));
			return true;
	}
}

float oWinControlGetFloat(HWND _hControl)
{
	oStd::mstring text;
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_FLOATBOX_SPINNER:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass through
		default: 
			GetWindowText(_hControl, text.c_str(), (int)text.capacity());
			break;
	}

	float f = 0.0f;
	if (!oStd::atof(text, &f))
		return std::numeric_limits<float>::quiet_NaN();
	return f;
}

bool oWinControlSetIcon(HWND _hControl, HICON _hIcon, int _SubItemIndex)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_ICON:
			if (_SubItemIndex != oInvalid)
				return oErrorSetLast(std::errc::invalid_argument, "Invalid _SubItemIndex");
			SendMessage(_hControl, STM_SETICON, (WPARAM)_hIcon, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

HICON oWinControlGetIcon(HWND _hControl, int _SubItemIndex)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_ICON:
			if (_SubItemIndex != oInvalid)
		{
				oErrorSetLast(std::errc::invalid_argument, "Invalid _SubItemIndex");
	return nullptr;
	}
			return oWinGetIcon(_hControl);
		default:
			oErrorSetLastBadType(_hControl, type);
			return nullptr;
	}
}

bool oWinControlIsChecked(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_CHECKBOX:
		case oGUI_CONTROL_RADIOBUTTON:
	{
			oErrorSetLast(0);
			LRESULT State = Button_GetState(_hControl);
			return (State & BST_CHECKED) == BST_CHECKED;
			}
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetChecked(HWND _hControl, bool _Checked)
{
	oWIN_CHECK(_hControl);
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_CHECKBOX:
		case oGUI_CONTROL_RADIOBUTTON:
			Button_SetCheck(_hControl, _Checked ? BST_CHECKED : BST_UNCHECKED);
	return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetRange(HWND _hControl, int _Min, int _Max)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			SendMessage(_hControl, TBM_SETRANGEMIN, TRUE, _Min);
			SendMessage(_hControl, TBM_SETRANGEMAX, TRUE, _Max);
			return true;
		case oGUI_CONTROL_PROGRESSBAR:
			SendMessage(_hControl, PBM_SETRANGE32, _Min, _Max);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlGetRange(HWND _hControl, int* _pMin, int* _pMax)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			if (_pMin) *_pMin = (int)SendMessage(_hControl, TBM_GETRANGEMIN, 0, 0);
			if (_pMax) *_pMax = (int)SendMessage(_hControl, TBM_GETRANGEMAX, 0, 0);
			return true;
		case oGUI_CONTROL_PROGRESSBAR:
		{
			PBRANGE pbr;
			SendMessage(_hControl, PBM_GETRANGE, 0, (LPARAM)&pbr);
			if (_pMin) *_pMin = pbr.iLow;
			if (_pMax) *_pMax = pbr.iHigh;
		}
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetRangePosition(HWND _hControl, int _Position, bool _bNotify /*=true*/)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE:
			SendMessage(_hControl, _bNotify ? TBM_SETPOSNOTIFY : TBM_SETPOS, 0, _Position);
			return true;
		case oGUI_CONTROL_PROGRESSBAR:
			SendMessage(_hControl, PBM_SETPOS, _Position, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

int oWinControlGetRangePosition(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER:
		case oGUI_CONTROL_SLIDER_WITH_TICKS:
		case oGUI_CONTROL_SLIDER_SELECTABLE: return (int)SendMessage(_hControl, TBM_GETPOS, 0, 0);
		case oGUI_CONTROL_PROGRESSBAR: return (int)SendMessage(_hControl, PBM_GETPOS, 0, 0);
		default: return oErrorSetLastBadType(_hControl, type);
	}
}

oAPI bool oWinControlSetTick(HWND _hControl, int _Position)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch(type)
	{
	case oGUI_CONTROL_SLIDER_WITH_TICKS:
		SendMessage(_hControl, TBM_SETTIC, 0, _Position);
		return true;
	default:
		return oErrorSetLastBadType(_hControl, type);
	}
}

oAPI bool oWinControlClearTicks(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch(type)
	{
	case oGUI_CONTROL_SLIDER_WITH_TICKS:
		SendMessage(_hControl, TBM_CLEARTICS, 1, 0);
		return true;
	default:
		return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetErrorState(HWND _hControl, bool _InErrorState)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_PROGRESSBAR:
			SendMessage(_hControl, PBM_SETSTATE, _InErrorState ? PBST_ERROR : PBST_NORMAL, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlGetErrorState(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_PROGRESSBAR:
			oErrorSetLast(0);
			return PBST_ERROR == SendMessage(_hControl, PBM_GETSTATE, 0, 0);
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlClampPositionToSelected(HWND _hControl)
{
	oGUI_CONTROL_TYPE type = oWinControlGetType(_hControl);
	switch (type)
	{
		case oGUI_CONTROL_SLIDER_SELECTABLE:
		{
			int Position = (int)SendMessage(_hControl, TBM_GETPOS, 0, 0);
			int SelectionStart = (int)SendMessage(_hControl, TBM_GETSELSTART, 0, 0);
			int SelectionEnd = (int)SendMessage(_hControl, TBM_GETSELEND, 0, 0);
			if (Position < SelectionStart)
				SendMessage(_hControl, TBM_SETPOS, TRUE, SelectionStart);
			else if (Position > SelectionEnd)
				SendMessage(_hControl, TBM_SETPOS, TRUE, SelectionEnd);
		}

		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}
