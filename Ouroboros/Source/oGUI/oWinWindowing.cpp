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
#include <oGUI/Windows/oWinWindowing.h>
#include <oGUI/Windows/oWinKey.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinStatusBar.h>
#include <oBase/timer.h>
#include <oBasis/oError.h> // @tony fixme

#undef interface
#include <oCore/windows/win_error.h>
#include <oCore/windows/win_util.h>
#include <oCore/windows/win_version.h>

#include <SetupAPI.h>
#include <Shlwapi.h>
#include <devguid.h>
#include <devpkey.h>
#include <windowsx.h>

using namespace ouro;

// Raw Input (WM_INPUT support) These are not defined anywhere I can find in
// Windows headers.

enum oUS_USAGE_PAGE_VALUES
{
	oUS_USAGE_PAGE_GENERIC_DESKTOP = 1,
	oUS_USAGE_PAGE_SIMULATION = 2,
	oUS_USAGE_PAGE_VIRTUAL_REALITY = 3,
	oUS_USAGE_PAGE_SPORT = 4,
	oUS_USAGE_PAGE_GAME = 5,
	oUS_USAGE_PAGE_GENERIC_DEVICE = 6,
	oUS_USAGE_PAGE_KEYBOARD = 7,
	oUS_USAGE_PAGE_LEDS = 8,
	oUS_USAGE_PAGE_BUTTON = 9,
};

enum oUS_USAGE
{
	oUS_USAGE_UNDEFINED = 0,
	oUS_USAGE_POINTER = 1,
	oUS_USAGE_MOUSE = 2,
	oUS_USAGE_RESERVED = 3,
	oUS_USAGE_JOYSTICK = 4,
	oUS_USAGE_GAMEPAD = 5,
	oUS_USAGE_KEYBOARD = 6,
	oUS_USAGE_KEYPAD = 7,
	oUS_USAGE_MULTIAXIS_CONTROLLER = 8,
	oUS_USAGE_TABLET_CONTROLS = 9,
};

#define oWIN_CHECK(_hWnd) do \
	{	if (!oWinExists(_hWnd)) return oErrorSetLast(std::errc::invalid_argument, "Invalid HWND 0x%x specified", _hWnd); \
		if (!oWinIsWindowThread(_hWnd)) return oErrorSetLast(std::errc::operation_not_permitted, "This function must be called on the window thread %d for HWND 0x%x", asdword(oStd::this_thread::get_id()), _hWnd); \
	} while (false)

#define oWIN_CHECK0(_hWnd) do \
	{	if (!oWinExists(_hWnd)) { oErrorSetLast(std::errc::invalid_argument, "Invalid HWND 0x%x specified", _hWnd); return 0; } \
		if (!oWinIsWindowThread(_hWnd)) { oErrorSetLast(std::errc::operation_not_permitted, "This function must be called on the window thread %d for HWND 0x%x", asdword(oStd::this_thread::get_id()), _hWnd); return 0; } \
	} while (false)

static const char* kRegisteredWindowMessages[] = 
{
	"oWM_INPUT_DEVICE_STATUS",
};
static_assert(oCOUNTOF(kRegisteredWindowMessages) == oWM_REGISTERED_COUNT, "array mismatch");

class oWinRegisteredMessages
{
public:
	oWinRegisteredMessages()
	{
		oFORI(i, kRegisteredWindowMessages)
			RegisteredMessages[i] = RegisterWindowMessage(kRegisteredWindowMessages[i]);
	}

	UINT Translate(UINT _uMsg)
	{
		if (oWinIsRegisteredMessage(_uMsg))
		{
			oFORI(i, RegisteredMessages)
				if (RegisteredMessages[i] == _uMsg)
					return (UINT)(oWM_REGISTERED_FIRST + i);
		}
		return _uMsg;
	}

private:
	UINT RegisteredMessages[oWM_REGISTERED_COUNT];
};

UINT oWinGetNativeRegisteredMessage(oWM _RegisteredMessage)
{
	return RegisterWindowMessage(kRegisteredWindowMessages[_RegisteredMessage - oWM_REGISTERED_FIRST]);
}

// copy from DEVPKEY.H
static const DEVPROPKEY oDEVPKEY_Device_DeviceDesc = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 2 };
static const DEVPROPKEY oDEVPKEY_Device_HardwareIds = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 3 };
static const DEVPROPKEY oDEVPKEY_Device_Class = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 9 };
static const DEVPROPKEY oDEVPKEY_Device_PDOName = { { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 16 };
static const DEVPROPKEY oDEVPKEY_Device_Parent = { { 0x4340a6c5, 0x93fa, 0x4706, { 0x97, 0x2c, 0x7b, 0x64, 0x80, 0x08, 0xa5, 0xa7 } }, 8 }; 
static const DEVPROPKEY oDEVPKEY_Device_InstanceId = { { 0x78c34fc8, 0x104a, 0x4aca, { 0x9e, 0xa4, 0x52, 0x4d, 0x52, 0x99, 0x6e, 0x57 } }, 256 };
static const DEVPROPKEY oDEVPKEY_Device_Siblings = { { 0x4340a6c5, 0x93fa, 0x4706, { 0x97, 0x2c, 0x7b, 0x64, 0x80, 0x08, 0xa5, 0xa7 } }, 10 };

#if 0
// if _EnumerateAll is false, then only existing/plugged in devices are searched.
// A single PnP device can support multiple device types, so this sets the 
// array of types the PnP device represents.

static char* oWinPnPEnumeratorToSetupClass(char* _StrDestination, size_t _SizeofStrDestination, const char* _PnPEnumerator)
{
	if (strlen(_PnPEnumerator) <= 4) return nullptr;
	size_t len = strlcpy(_StrDestination, _PnPEnumerator + 4, _SizeofStrDestination);
	if (len >= _SizeofStrDestination) return nullptr;
	char* hash = strrchr(_StrDestination, '#');
	if (!hash) return nullptr;
	*hash = '\0';
	char* end = _StrDestination + len;
	ouro::transform(_StrDestination, end, _StrDestination, [=](char c) { return (c == '#') ? '\\' : c; });
	ouro::transform(_StrDestination, end, _StrDestination, ouro::toupper<const char&>);
	char* first_slash = strchr(_StrDestination, '\\');
	if (!first_slash) return nullptr;
	*first_slash = '\0';
	return _StrDestination;
}

template<size_t size> char* oWinPnPEnumeratorToSetupClass(char (&_StrDestination)[size], const char* _PnPEnumerator) { return oWinPnPEnumeratorToSetupClass(_StrDestination, size, _PnPEnumerator); }
template<typename charT, size_t capacity> char* oWinPnPEnumeratorToSetupClass(ouro::fixed_string<charT, capacity>& _StrDestination, const char* _PnPEnumerator) { return oWinPnPEnumeratorToSetupClass(_StrDestination, _StrDestination.capacity(), _PnPEnumerator); }
#endif

ouro::input::type oWinGetDeviceTypeFromClass(const char* _Class)
{
	static const char* DeviceClass[ouro::input::type_count] = 
	{
		"Unknown",
		"Keyboard",
		"Mouse",
		"Controller",
		"Control", // won't ever get this from HW config
		"Skeleton",
		"Voice",
		"Touch", // not sure what this is called generically
	};
	static_assert(oCOUNTOF(DeviceClass) == ouro::input::type_count, "array mismatch");
	for (int i = 0; i < oCOUNTOF(DeviceClass); i++)
		if (!strcmp(DeviceClass[i], _Class))
			return (ouro::input::type)i;
	return ouro::input::unknown;
}

static bool oWinSetupGetString(ouro::mstring& _StrDestination, HDEVINFO _hDevInfo, SP_DEVINFO_DATA* _pSPDID, const DEVPROPKEY* _pPropKey)
{
	DEVPROPTYPE dpType;
	DWORD size = 0;
	SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, nullptr, 0, &size, 0);
	if (size)
	{
		ouro::mwstring buffer;
		oASSERT(buffer.capacity() > (size / sizeof(ouro::mwstring::char_type)), "");
		oVB(SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, (BYTE*)buffer.c_str(), size, &size, 0) && dpType == DEVPROP_TYPE_STRING);
		_StrDestination = buffer;
	}
	else
		_StrDestination.clear();

	return true;
}

// _NumStrings must be initialized to maximum capacity
static bool oWinSetupGetStringList(ouro::mstring* _StrDestination, size_t& _NumStrings, HDEVINFO _hDevInfo, SP_DEVINFO_DATA* _pSPDID, const DEVPROPKEY* _pPropKey)
{
	DEVPROPTYPE dpType;
	DWORD size = 0;
	SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, nullptr, 0, &size, 0);
	if (size)
	{
		ouro::xlwstring buffer;
		oASSERT(buffer.capacity() > size, "");
		oVB(SetupDiGetDevicePropertyW(_hDevInfo, _pSPDID, _pPropKey, &dpType, (BYTE*)buffer.c_str(), size, &size, 0) && dpType != DEVPROP_TYPE_STRING_LIST);

		const wchar_t* c = buffer.c_str();
		size_t i = 0;
		for (; i < _NumStrings; i++)
		{
			_StrDestination[i] = c;
			c += wcslen(c) + 1;

			if (!*c)
			{
				_NumStrings = i + 1;
				break;
			}
		}

		if (i > _NumStrings)
		{
			while (*c)
			{
				c += wcslen(c) + 1;
				i++;
			}

			_NumStrings = i + 1;
			return oErrorSetLast(std::errc::no_buffer_space, "not enough strings");
		}
	}
	else
		_NumStrings = 0;

	return true;
}

struct oWINDOWS_HID_DESC
{
	oWINDOWS_HID_DESC()
		: Type(ouro::input::unknown)
		, DevInst(0)
	{}

	ouro::input::type Type;
	DWORD DevInst;
	ouro::mstring ParentDeviceInstancePath;
	ouro::mstring DeviceInstancePath;
};

static bool oWinEnumInputDevices(bool _EnumerateAll, const char* _Enumerator, const std::function<void(const oWINDOWS_HID_DESC& _HIDDesc)>& _Visitor)
{
	if (!_Visitor)
		return oErrorSetLast(std::errc::invalid_argument, "Must specify a visitor.");

	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	ouro::finally([=]
	{ 
		if (hDevInfo != INVALID_HANDLE_VALUE) 
			SetupDiDestroyDeviceInfoList(hDevInfo);
	});

	DWORD dwFlag = DIGCF_ALLCLASSES;
	if (!_EnumerateAll)
		dwFlag |= DIGCF_PRESENT;

	hDevInfo = SetupDiGetClassDevsA(nullptr, _Enumerator, nullptr, dwFlag);
	oVB(INVALID_HANDLE_VALUE != hDevInfo);

	SP_DEVINFO_DATA SPDID;
	memset(&SPDID, 0, sizeof(SPDID));
	SPDID.cbSize = sizeof(SPDID);
	int DeviceIndex = 0;
	bool KnownTypeFound = false;

	oWINDOWS_HID_DESC HIDDesc;

	ouro::mstring KinectCameraSibling;
	ouro::mstring KinectCameraParent;

	while (SetupDiEnumDeviceInfo(hDevInfo, DeviceIndex, &SPDID))
	{
		DeviceIndex++;

		ouro::mstring ClassValue;
		if (!oWinSetupGetString(ClassValue, hDevInfo, &SPDID, &oDEVPKEY_Device_Class))
			return false; // pass through error

		if (!oWinSetupGetString(HIDDesc.ParentDeviceInstancePath, hDevInfo, &SPDID, &oDEVPKEY_Device_Parent))
			return false; // pass through error

		// Fall back on generic desc string for Kinect, which apparently is in a 
		// class by itself at the moment.
		if (!strcmp("Microsoft Kinect", ClassValue))
		{
			if (!oWinSetupGetString(ClassValue, hDevInfo, &SPDID, &oDEVPKEY_Device_DeviceDesc))
				return false; // pass through error

			if (strstr(ClassValue, "Camera"))
			{
				ClassValue = "Skeleton";
				HIDDesc.DeviceInstancePath = HIDDesc.ParentDeviceInstancePath;

				ouro::mstring Strings[2];
				size_t Capacity = oCOUNTOF(Strings);
				if (!oWinSetupGetStringList(Strings, Capacity, hDevInfo, &SPDID, &oDEVPKEY_Device_Siblings) || Capacity != 1)
					return false; // pass through error

				KinectCameraSibling = Strings[0];
				KinectCameraParent = HIDDesc.ParentDeviceInstancePath;
			}

			else if (strstr(ClassValue, "Audio"))
			{
				ClassValue = "Voice";
				if (HIDDesc.ParentDeviceInstancePath == KinectCameraSibling)
					HIDDesc.DeviceInstancePath = KinectCameraParent;

				else
					HIDDesc.DeviceInstancePath = HIDDesc.ParentDeviceInstancePath;
			}
			else
				ClassValue = "Unknown";
		}

		else
		{
			if (!oWinSetupGetString(HIDDesc.DeviceInstancePath, hDevInfo, &SPDID, &oDEVPKEY_Device_InstanceId))
				return false; // pass through error
		}

		// There are often meta-classes to be ignored, so don't flag unknown unless
		// every other type is not present.
		HIDDesc.Type = oWinGetDeviceTypeFromClass(ClassValue);
		HIDDesc.DevInst = SPDID.DevInst;

		_Visitor(HIDDesc);
	}

	return true;
}

// Translate a PnP enumerator name (DEV_BROADCAST_DEVICEINTERFACE_A::dbcc_name)
// to one or more unique names based on the ouro::input_device_status::TYPEs represented
// by the device. Devices now are often hybrid, such as a KB/mouse combo USB or 
// a voice/skeleton Kinect device. This uses SetupAPI to do the discovery.
static bool oWinEnumInputDevices(bool _EnumerateAll, const std::function<void(const oWINDOWS_HID_DESC& _HIDDesc)>& _Visitor)
{
	return oWinEnumInputDevices(_EnumerateAll, "HID", _Visitor) && oWinEnumInputDevices(_EnumerateAll, "USB", _Visitor);
}

struct oWIN_DEVICE_CHANGE_CONTEXT
{
	oWIN_DEVICE_CHANGE_CONTEXT()
	{
		LastChangeTimestamp.fill(0);
	}

	std::vector<RAWINPUTDEVICELIST> RawInputs;
	std::vector<mstring> RawInputInstanceNames;
	std::array<unsigned int, ouro::input::status_count> LastChangeTimestamp;
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
	oVB(RegisterRawInputDevices(RID, oCOUNTOF(RID), sizeof(RAWINPUTDEVICE)));

	// Windows sends KB and mouse device notifications automatically, so make up the
	// difference here.
	oVERIFY_R(oWinEnumInputDevices(false, [&](const oWINDOWS_HID_DESC& _HIDDesc)
	{
		switch(_HIDDesc.Type)
		{
			case ouro::input::keyboard: case ouro::input::mouse: case ouro::input::unknown: break;
			default: SendMessage(_hWnd, oWM_INPUT_DEVICE_CHANGE, MAKEWPARAM(_HIDDesc.Type, ouro::input::ready), (LPARAM)_HIDDesc.DeviceInstancePath.c_str());
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

static ouro::input::type oWinGetTypeFromRIM(DWORD _dwRIMType)
{
	switch (_dwRIMType)
	{
		case RIM_TYPEKEYBOARD: return ouro::input::keyboard;
		case RIM_TYPEMOUSE: return ouro::input::mouse;
		default: break;
	}
	return ouro::input::unknown;
}

static bool oWinTranslateDeviceChange(HWND _hWnd, WPARAM _wParam, LPARAM _lParam, HDEVICECHANGE _hDeviceChance)
{
	oWIN_DEVICE_CHANGE_CONTEXT* ctx = (oWIN_DEVICE_CHANGE_CONTEXT*)_hDeviceChance;
	ouro::input::type Type = ouro::input::unknown;
	ouro::input::status Status = _wParam == GIDC_ARRIVAL ? ouro::input::ready : ouro::input::not_ready;
	mstring InstanceName;

	switch (_wParam)
	{
		// We have full and direct information.
		case GIDC_ARRIVAL:
		{
			RID_DEVICE_INFO RIDDI;
			RIDDI.cbSize = sizeof(RIDDI);
			UINT Size = sizeof(RIDDI);
			UINT NameCapacity = as_uint(InstanceName.capacity());
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
				UINT Capacity = as_uint(ctx->RawInputInstanceNames[i].capacity());
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

static BOOL CALLBACK oWinEnumWindowsProc(HWND _hWnd, LPARAM _lParam)
{
	return (*(std::function<bool(HWND _hWnd)>*)_lParam)(_hWnd) ? TRUE : FALSE;
}

void oWinEnumWindows(const std::function<bool(HWND _hWnd)>& _Enumerator)
{
	EnumWindows(oWinEnumWindowsProc, (LPARAM)&_Enumerator);
}

bool oWinGetProcessTopWindowAndThread(ouro::process::id _ProcessID
	, HWND* _pHWND
	, std::thread::id* _pThreadID
	, const char* _pWindowName)
{
	HWND Hwnd = 0;
	unsigned int ThreadID = 0;

	unsigned int PID = *(unsigned int*)&_ProcessID;

	oWinEnumWindows(
		[&](HWND _Hwnd)->bool
	{
		unsigned int HWNDProcessID;
		auto NewThreadID = GetWindowThreadProcessId(_Hwnd, (DWORD*)&HWNDProcessID);

		if (HWNDProcessID != PID)
			return true;

		// Look for windows that might get injected into our process that we would never want to target
		const char* WindowsToSkip[] = {"Default IME", "MSCTFIME UI"};
		ouro::mstring WindowText;
		// For some reason in release builds Default IME can take a while to respond to this. so wait longer unless the app appears to be hung.
		if (SendMessageTimeout(_Hwnd, WM_GETTEXT, ouro::mstring::Capacity - 1, (LPARAM)WindowText.c_str(), SMTO_ABORTIFHUNG, 2000, nullptr) == 0)
		{
			WindowText = "";
		}

		if (!WindowText.empty())
		{
			oFORI(i, WindowsToSkip)
			{
				if( 0 == strcmp(WindowText.c_str(), WindowsToSkip[i]) )
				{
					return true;
				}
			}
		}

		// If the caller specified a window name make certain this is the right window or else keep searching
		if(_pWindowName && 0 != strcmp(_pWindowName, WindowText))
			return true;

		Hwnd = _Hwnd;
		ThreadID = NewThreadID;
		return false;
	});

	if(!Hwnd)
		return oErrorSetLast(std::errc::no_such_process);

	*_pHWND = Hwnd;
	*_pThreadID = astid(ThreadID);
	return true;
}

// @tony: Confirmation of hWnd being on the specified thread is disabled for 
// now... I think it might be trying to access the HWND before it's fully
// constructed. First get the massive integration done, then come back to this.

#define oWINVP(_hWnd) \
	if (!oWinExists(_hWnd)) \
		{ oErrorSetLast(std::errc::invalid_argument, "Invalid HWND %p specified", _hWnd); return nullptr; } \
	oASSERT(oWinIsWindowThread(_hWnd), "This function must be called on the window thread %d for %p", asdword(oStd::this_thread::get_id()), _hWnd)

inline bool oErrorSetLastBadType(HWND _hControl, ouro::control_type::value _Type) { return oErrorSetLast(std::errc::invalid_argument, "The specified %s %p (%d) is not valid for this operation", as_string(_Type), _hControl, GetDlgCtrlID(_hControl)); }

inline bool oErrorSetLastBadIndex(HWND _hControl, ouro::control_type::value _Type, int _SubItemIndex) { return oErrorSetLast(std::errc::invalid_argument, "_SubItemIndex %d was not found in %s %p (%d)", _SubItemIndex, as_string(_Type), _hControl, GetDlgCtrlID(_hControl)); }

inline HWND oWinControlGetBuddy(HWND _hControl) { return (HWND)SendMessage(_hControl, UDM_GETBUDDY, 0, 0); }

static DWORD oWinGetStyle(ouro::window_style::value _Style, bool _HasParent)
{
	switch (_Style)
	{
		case ouro::window_style::borderless: return _HasParent ? WS_CHILD : WS_POPUP;
		case ouro::window_style::dialog: return WS_CAPTION;
		case ouro::window_style::fixed: 
		case ouro::window_style::fixed_with_menu:
		case ouro::window_style::fixed_with_statusbar:
		case ouro::window_style::fixed_with_menu_and_statusbar: return WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
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
	oWinRegisteredMessages* pRegisteredMessages;
	HDEVICECHANGE hDeviceChange;
	LONG_PTR RestoredPosition; // MAKELPARAM(x,y)
	LONG_PTR RestoredSize; // MAKELPARAM(w,h)
	LONG_PTR PreviousState; // ouro::window_state::value
	LONG_PTR PreviousStyle; // ouro::window_style::value
	LONG_PTR ExtraFlags;
	intptr_t TempCallCounter;
	intptr_t LastShowTimestamp;
};

// This is set when the handler should ignore the event because its about to be 
// followed up with the same event with more meaningful data.
#define oWNDEXTRA_FLAGS_TEMP_CHANGE (0x1)

#define oGWLP_MENU (offsetof(oWndExtra, hMenu))
#define oGWLP_STATUSBAR (offsetof(oWndExtra, hStatusBar))
#define oGWLP_REGISTERED_MESSAGES (offsetof(oWndExtra, pRegisteredMessages))
#define oGWLP_DEVICE_CHANGE (offsetof(oWndExtra, hDeviceChange))
#define oGWLP_RESTORED_POSITION (offsetof(oWndExtra, RestoredPosition)) // use GET_X_LPARAM and GET_Y_LPARAM to decode
#define oGWLP_RESTORED_SIZE (offsetof(oWndExtra, RestoredSize)) // use GET_X_LPARAM and GET_Y_LPARAM to decode
#define oGWLP_PREVIOUS_STATE (offsetof(oWndExtra, PreviousState)) // ouro::window_state::value
#define oGWLP_PREVIOUS_STYLE (offsetof(oWndExtra, PreviousStyle)) // ouro::window_style::value
#define oGWLP_EXTRA_FLAGS (offsetof(oWndExtra, ExtraFlags))
#define oGWLP_TEMP_CALL_COUNTER (offsetof(oWndExtra, TempCallCounter))
#define oGWLP_LAST_SHOW_TIMESTAMP (offsetof(oWndExtra, LastShowTimestamp))

#define oEF_RENDER_TARGET (1<<0)
#define oEF_FULLSCREEN_EXCLUSIVE (1<<1)
#define oEF_FULLSCREEN_COOPERATIVE (1<<2)
#define oEF_NO_SAVE_RESTORED_POSITION_SIZE (1<<3)
#define oEF_ALT_F4_ENABLED (1<<4)
#define oEF_THREAD_OWNER (1<<5)

HWND oWinCreate(HWND _hParent
	, const char* _Title
	, ouro::window_style::value _Style
	, const int2& _ClientPosition
	, const int2& _ClientSize
	, WNDPROC _Wndproc
	, void* _pInit
	, void* _pThis)
{
	sstring ClassName;
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
	oVB(0 != RegisterClassEx(&wc) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS);

	// Resolve initial position and size
	int2 NewPosition = _ClientPosition;
	int2 NewSize = _ClientSize;
	if (NewPosition.x == oDEFAULT || NewPosition.y == oDEFAULT || NewSize.x == oDEFAULT || NewSize.y == oDEFAULT)
	{
		ouro::display::info di = ouro::display::get_info(ouro::display::primary_id());
		const RECT rPrimaryWorkarea = oWinRectWH(int2(di.workarea_x, di.workarea_y), int2(di.workarea_width, di.workarea_height));
		const int2 DefaultSize = oWinRectSize(rPrimaryWorkarea) / 4; // 25% of parent window by default
		NewSize = ouro::resolve_rect_size(NewSize, DefaultSize);
		const RECT rCenteredClient = oWinRect(ouro::resolve_rect(oRect(rPrimaryWorkarea), int2(0, 0), NewSize, ouro::alignment::middle_center, false));
		const int2 DefaultPosition = oWinRectPosition(rCenteredClient);
		NewPosition = ouro::resolve_rect_position(NewPosition, DefaultPosition);
	}

	const DWORD dwInitialStyleEx = WS_EX_ACCEPTFILES|WS_EX_APPWINDOW;
	const DWORD dwInitialStyle = oWinGetStyle(_Style, false);

	RECT rWindow = oWinRectWH(NewPosition, NewSize);
	oVB(AdjustWindowRectEx(&rWindow, dwInitialStyle, false, dwInitialStyleEx));
	oWIN_CREATESTRUCT wcs;
	wcs.Shape.state = ouro::window_state::hidden;
	wcs.Shape.style = _Style;
	wcs.Shape.client_position = NewPosition;
	wcs.Shape.client_size = NewSize;
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
			oTHROW(protocol_error, "CreateWindowEx returned a null HWND (failure condition) for '%s' but GetLastError is S_OK. This implies that user handling of a WM_CREATE message failed, so start looking there.", oSAFESTRN(_Title));
		else
			throw ouro::windows::error();
	}

	// only top-level windows should be allowed to quit the message loop
	SetWindowLongPtr(hWnd, oGWLP_EXTRA_FLAGS, GetWindowLongPtr(hWnd, oGWLP_EXTRA_FLAGS) | oEF_THREAD_OWNER);

	oTRACE("HWND 0x%x '%s' running on thread %d (0x%x)"
		, hWnd
		, oSAFESTRN(_Title)
		, asdword(oStd::this_thread::get_id())
		, asdword(oStd::this_thread::get_id()));

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
	return as_size_t(written) < _SizeofStrDestination ? _StrDestination : nullptr;
}

bool oWinIsClass(HWND _hWnd, WNDPROC _Wndproc)
{
	sstring ClassName, ExpectedClassName;
	oVERIFY_R(oWinMakeClassName(ExpectedClassName, _Wndproc));
	oVERIFY_R(GetClassName(_hWnd, ClassName.c_str(), as_int(ClassName.capacity())));
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
			ouro::window_shape s = oWinGetShape(_hWnd);
			SetWindowLongPtr(_hWnd, oGWLP_RESTORED_POSITION, (LONG_PTR)MAKELPARAM(s.client_position.x, s.client_position.y));
			SetWindowLongPtr(_hWnd, oGWLP_RESTORED_SIZE, (LONG_PTR)MAKELPARAM(s.client_size.x, s.client_size.y));
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
				SetWindowLongPtr(_hWnd, oGWLP_REGISTERED_MESSAGES, (LONG_PTR)new oWinRegisteredMessages);
				SetWindowLongPtr(_hWnd, oGWLP_DEVICE_CHANGE, (LONG_PTR)oWinDeviceChangeCreate());
				SetWindowLongPtr(_hWnd, oGWLP_MENU, (LONG_PTR)CreateMenu());
				SetWindowLongPtr(_hWnd, oGWLP_STATUSBAR, (LONG_PTR)oWinStatusBarCreate(_hWnd, (HMENU)0x00005747));
				oVERIFY(oWinShowStatusBar(_hWnd, ouro::has_statusbar(wcs->Shape.style)));

				if (ouro::has_statusbar(wcs->Shape.style))
					oVB(SetWindowPos(_hWnd, 0, cs->x, cs->y, cs->cx, cs->cy + oWinGetStatusBarHeight(_hWnd), SWP_NOZORDER|SWP_FRAMECHANGED|SWP_NOMOVE));

				oWinSaveRestoredPosSize(_hWnd);
				oWinSetTempChange(_hWnd, wcs->Shape.state != ouro::window_state::maximized);
				oWinShowMenu(_hWnd, ouro::has_menu(wcs->Shape.style));
				oWinSetTempChange(_hWnd, false);

				SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)wcs->pThis);
				break;
			}

			case WM_INITDIALOG:
				// dialogs don't use CREATESTRUCT, so assume it's directly the context
				SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)_lParam);
				break;

			case WM_DESTROY:
			{
				// If GetMenu returns the menu, then Windows will handle its destruction
				if (!::GetMenu(_hWnd))
					DestroyMenu((HMENU)SetWindowLongPtr(_hWnd, oGWLP_MENU, (LONG_PTR)nullptr));

				// As a child window, the status bar is implicitly destroyed.

				// minimizes "randomly set focus to some window other than parent"
				// @tony: Revisit this: this causes a child window to be reset in 
				// position and looks ugly. It may be true that position needs to be 
				// set here as well.
				//oWinSetOwner(_hWnd, nullptr);

				oWinDeviceChangeDestroy((HDEVICECHANGE)GetWindowLongPtr(_hWnd, oGWLP_DEVICE_CHANGE));
				SetWindowLongPtr(_hWnd, oGWLP_DEVICE_CHANGE, (LONG_PTR)nullptr);

				oWinRegisteredMessages* pRegisteredMessages = (oWinRegisteredMessages*)GetWindowLongPtr(_hWnd, oGWLP_REGISTERED_MESSAGES);
				delete pRegisteredMessages;
				SetWindowLongPtr(_hWnd, oGWLP_REGISTERED_MESSAGES, (LONG_PTR)nullptr);

				if (oWinIsThreadOwner(_hWnd))
					PostQuitMessage(0);
				break;
			}

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

				ouro::window_shape s;
				switch (_wParam)
				{
					case SC_MAXIMIZE:
						s.state = ouro::window_state::maximized;
						oWinSetShape(_hWnd, s);
						return 0;
					case SC_MINIMIZE:
						s.state = ouro::window_state::minimized;
						oWinSetShape(_hWnd, s);
						return 0;
					case SC_RESTORE:
					{
						ouro::window_shape Old = oWinGetShape(_hWnd);
						if (Old.state == ouro::window_state::minimized)
						{
							s.state = (ouro::window_state::value)GetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STATE);
							if (s.state == ouro::window_state::hidden)
								s.state = ouro::window_state::restored;
						}
						else 
							s.state = ouro::window_state::restored;

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
					SetWindowLongPtr(_hWnd, oGWLP_LAST_SHOW_TIMESTAMP, (ULONG_PTR)ouro::timer::now_ms());
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

UINT oWinTranslateMessage(HWND _hWnd, UINT _uMsg)
{
	oWinRegisteredMessages* p = (oWinRegisteredMessages*)GetWindowLongPtr(_hWnd, oGWLP_REGISTERED_MESSAGES);
	return p ? p->Translate(_uMsg) : _uMsg;
}

std::thread::id oWinGetWindowThread(HWND _hWnd)
{
	return astid(GetWindowThreadProcessId(_hWnd, nullptr));
}

bool oWinIsOpaque(HWND _hWnd)
{
	// @tony: I can't find API to ask about the opacity of an HWND, so just wait 
	// for a while.
	oWIN_CHECK(_hWnd);
	static const intptr_t kFadeInTime = 200;
	intptr_t LastShowTimestamp = (intptr_t)GetWindowLongPtr(_hWnd, oGWLP_LAST_SHOW_TIMESTAMP);
	if (LastShowTimestamp < 0)
		return false;
	intptr_t Now = ouro::timer::now_ms();
	return (LastShowTimestamp + kFadeInTime) < Now;
}

bool oWinRegisterTouchEvents(HWND _hWnd, bool _Registered)
{
	#ifdef oWINDOWS_HAS_REGISTERTOUCHWINDOW
		ouro::windows::version::value v = ouro::windows::get_version();
		if (v >= ouro::windows::version::win7)
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

void oWinAccelFromHotKeys(ACCEL* _pAccels, const ouro::basic_hotkey_info* _pHotKeys, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		ACCEL& a = _pAccels[i];
		const ouro::basic_hotkey_info& h = _pHotKeys[i];
		a.fVirt = FVIRTKEY;
		if (h.alt_down) a.fVirt |= FALT;
		if (h.ctrl_down) a.fVirt |= FCONTROL;
		if (h.shift_down) a.fVirt |= FSHIFT;
		a.key = oWinKeyFromKey(h.hotkey) & 0xffff;
		a.cmd = h.id;
	}
}

void oWinAccelToHotKeys(ouro::basic_hotkey_info* _pHotKeys, const ACCEL* _pAccels, size_t _NumHotKeys)
{
	for (size_t i = 0; i < _NumHotKeys; i++)
	{
		const ACCEL& a = _pAccels[i];
		ouro::basic_hotkey_info& h = _pHotKeys[i];
		oASSERT(a.fVirt & FVIRTKEY, "");
		oWINKEY_CONTROL_STATE dummy;
		h.hotkey = oWinKeyToKey(a.key);
		h.alt_down = !!(a.fVirt & FALT);
		h.ctrl_down = !!(a.fVirt & FCONTROL);
		h.shift_down = !!(a.fVirt & FSHIFT);
		h.id = a.cmd;
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
			// @tony: Hmm, this won't behave well with multiple tab controls in
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
				// @tony: Not sure what this should be because the docs say it 
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
		if (n == 0 || msg.message == oWM_QUIT)
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

	oVB(::SetMenu(_hWnd, _Show ? hMenu : nullptr));
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
		oVB(RedrawWindow(hStatusBar, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW));
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
	oVB(SetWindowLongPtr(_hWnd, GWLP_HWNDPARENT, (LONG_PTR)_hOwner) || GetLastError() == S_OK);
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

bool oWinIsThreadOwner(HWND _hWnd)
{
	oWIN_CHECK(_hWnd);
	return !!(GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) & oEF_THREAD_OWNER);
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

bool oWinSetIsFullscreenExclusive(HWND _hWnd, bool _IsFullscreenExclusive)
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
	// @tony: This shouldn't be true, there's several calls below so thus
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
	oVB(IsWindowEnabled(_hWnd));
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
	oVB(GetWindowRect(_hWnd, &r));
	return !!::SetWindowPos(_hWnd, _AlwaysOnTop ? HWND_TOPMOST : HWND_TOP, r.left, r.top, oWinRectW(r), oWinRectH(r), IsWindowVisible(_hWnd) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW);
}

bool oWinGetClientRect(HWND _hWnd, RECT* _pRect)
{
	oWIN_CHECK(_hWnd);

	if (!_pRect)
		return oErrorSetLast(std::errc::invalid_argument);

	// Discount status bar from dimensions
	oVB(GetClientRect(_hWnd, _pRect));
	_pRect->bottom = __max(_pRect->top, _pRect->bottom - oWinGetStatusBarHeight(_hWnd));

	// Translate to proper offset
	POINT p = { _pRect->left, _pRect->top };
	HWND hParent = GetParent(_hWnd);
	if (!hParent)
		oVB(ClientToScreen(_hWnd, &p));
	else
	{
		SetLastError(S_OK); // differentiate between 0-means-failure, and 0,0 adjustment
		oVB(MapWindowPoints(_hWnd, hParent, &p, 1) || GetLastError() == S_OK);
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
		ouro::display::info di = ouro::display::get_info(oWinGetDisplayId(_hWnd));
		rParent = oWinRectWH(int2(di.workarea_x, di.workarea_y), int2(di.workarea_width, di.workarea_height));
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

ouro::window_state::value oWinGetState(HWND _hWnd)
{
	if (!oWinExists(_hWnd)) return ouro::window_state::invalid;
	
	LONG_PTR style = GetWindowLongPtr(_hWnd, GWL_STYLE);
	if (!(style & WS_VISIBLE)) return ouro::window_state::hidden;

	LONG_PTR extra = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
	if (extra & (oEF_FULLSCREEN_EXCLUSIVE|oEF_FULLSCREEN_COOPERATIVE)) return ouro::window_state::fullscreen;
	
	if (IsIconic(_hWnd)) return ouro::window_state::minimized;
	if (IsZoomed(_hWnd)) return ouro::window_state::maximized;

	return ouro::window_state::restored;
}

static ouro::window_style::value oWinGetStyle(HWND _hWnd)
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
		if (HasMenu && HasStatusBar) return ouro::window_style::sizable_with_menu_and_statusbar;
		else if (HasMenu) return ouro::window_style::sizable_with_menu;
		else if (HasStatusBar) return ouro::window_style::sizable_with_statusbar;
		else return ouro::window_style::sizable;
	}
	else if (oSET(oFIXED_STYLE))
	{
		if (HasMenu && HasStatusBar) return ouro::window_style::fixed_with_menu_and_statusbar;
		else if (HasMenu) return ouro::window_style::fixed_with_menu;
		else if (HasStatusBar) return ouro::window_style::fixed_with_statusbar;
		else return ouro::window_style::fixed;
	}

	LONG_PTR exstyle = GetWindowLongPtr(_hWnd, GWL_EXSTYLE);
	if (exstyle & WS_EX_WINDOWEDGE) return ouro::window_style::dialog;
	return ouro::window_style::borderless;
	
	#undef oFIXED_STYLE
	#undef oSET
}

ouro::window_shape oWinGetShape(HWND _hWnd)
{
	ouro::window_shape s;
	RECT rClient;
	oVERIFY(oWinGetClientRect(_hWnd, &rClient));
	s.client_size = oWinRectSize(rClient);
	s.client_position = oWinRectPosition(rClient);
	s.state = oWinGetState(_hWnd);
	s.style = oWinGetStyle(_hWnd);
	return s;
}

bool oWinSetShape(HWND _hWnd, const ouro::window_shape& _Shape)
{
	oWIN_CHECK(_hWnd);

	ouro::window_shape New = _Shape;
	ouro::window_shape Old = oWinGetShape(_hWnd);
	oASSERT(Old.state != ouro::window_state::invalid && Old.style != ouro::window_style::default_style, "");

	if (New.state == ouro::window_state::invalid)
		New.state = Old.state;

	if (New.style == ouro::window_style::default_style)
		New.style = Old.style;

	if (ouro::has_statusbar(New.style) && oWinIsRenderTarget(_hWnd))
		return oErrorSetLast(std::errc::protocol_error, "HWND 0x%x is marked as a render target, disallowing status bar styles to be set", _hWnd);

	if (Old.state == ouro::window_state::restored)
		oWinSaveRestoredPosSize(_hWnd);

	finally f([&] {SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) &~ oEF_NO_SAVE_RESTORED_POSITION_SIZE);});
	if (New.state == ouro::window_state::fullscreen || New.state == ouro::window_state::minimized || New.state == ouro::window_state::maximized)
		SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) | oEF_NO_SAVE_RESTORED_POSITION_SIZE);

	if (Old.state != New.state)
		SetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STATE, (LONG_PTR)Old.state);

	if (Old.state != ouro::window_state::fullscreen)
		SetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STYLE, (LONG_PTR)Old.style);

	if (New.state == ouro::window_state::fullscreen)
	{
		New.style = ouro::window_style::borderless;
		LONG_PTR extra = GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS);
		if (New.state == ouro::window_state::fullscreen)
			extra |= oEF_FULLSCREEN_COOPERATIVE;
		else
			extra &=~ oEF_FULLSCREEN_COOPERATIVE;
		SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, extra);
	}
	else if (Old.state == ouro::window_state::fullscreen)
	{
		New.style = (ouro::window_style::value)GetWindowLongPtr(_hWnd, oGWLP_PREVIOUS_STYLE);
		SetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS, GetWindowLongPtr(_hWnd, oGWLP_EXTRA_FLAGS) & ~(oEF_FULLSCREEN_COOPERATIVE|oEF_FULLSCREEN_EXCLUSIVE));
	}

	if (New.state == ouro::window_state::restored)
	{
		LONG_PTR lParam = GetWindowLongPtr(_hWnd, oGWLP_RESTORED_POSITION);
		const int2 RestoredClientPosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		New.client_position = ouro::resolve_rect_position(New.client_position, RestoredClientPosition);
		lParam = GetWindowLongPtr(_hWnd, oGWLP_RESTORED_SIZE);
		const int2 RestoredClientSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		New.client_size = ouro::resolve_rect_position(New.client_size, RestoredClientSize);
	}

	else if (New.state == ouro::window_state::fullscreen)
	{
		ouro::display::info di = ouro::display::get_info(oWinGetDisplayId(_hWnd));
		New.client_position = int2(di.x, di.y);
		New.client_size = int2(di.mode.width, di.mode.height);
	}

	else
	{
		New.client_position = ouro::resolve_rect_position(New.client_position, Old.client_position);
		New.client_size = ouro::resolve_rect_position(New.client_size, Old.client_size);
	}

	int StatusBarHeight = 0;
	{
		RECT rStatusBar;
		GetClientRect(oWinGetStatusBar(_hWnd), &rStatusBar);
		StatusBarHeight = oWinRectH(rStatusBar);
	}

	DWORD dwAllFlags = (DWORD)GetWindowLongPtr(_hWnd, GWL_STYLE);
	if (Old.style != New.style)
	{
		// Change only the bits we mean to change and preserve the others
		const bool HasParent = !!GetParent(_hWnd);
		DWORD dwCurrentStyleFlags = oWinGetStyle(Old.style, HasParent);
		dwAllFlags &=~ dwCurrentStyleFlags;
		dwAllFlags |= oWinGetStyle(New.style, HasParent);

		// When in maximized state and doing nothing more than toggling status bar
		// visibility, the client area has changed, so spoof a WM_SIZE to notify
		// the system of that change. If the menu changes, then allow the WM_SIZE to 
		// be sent by oWinShowMenu.
		oVERIFY(oWinShowStatusBar(_hWnd, ouro::has_statusbar(New.style)));

		if (Old.state == ouro::window_state::maximized && New.state == ouro::window_state::maximized 
			&& ouro::has_statusbar(New.style) != ouro::has_statusbar(Old.style)
			&& ouro::has_menu(New.style) == ouro::has_menu(Old.style))
		{
			int2 NewMaxSize = Old.client_size;
			if (ouro::has_statusbar(New.style))
				NewMaxSize.y -= StatusBarHeight;
			else
				NewMaxSize.y += StatusBarHeight;
			SendMessage(_hWnd, WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(NewMaxSize.x, NewMaxSize.y));
		}

		// This will send a temp WM_SIZE event. Another will be sent below, so 
		// squelch this one from making it all the way through to an ouro::event_type::value.
		// However, during a style change to a maximized window, that affects the 
		// menu other calls won't be made, so let this be the authority in that case.
		oWinSetTempChange(_hWnd, !(Old.state == ouro::window_state::maximized && New.state == ouro::window_state::maximized));
		oWinShowMenu(_hWnd, ouro::has_menu(New.style));
		oWinSetTempChange(_hWnd, false);
	}

	// Resolve position and size to a rectangle. Add extra room for the status bar.
	RECT r = oWinRectWH(New.client_position, New.client_size);
	if (ouro::has_statusbar(New.style))
		r.bottom += StatusBarHeight;

	// @tony: are these bit-clears needed?
	if (New.state != ouro::window_state::maximized)
		dwAllFlags &=~ WS_MAXIMIZE;

	if (New.state != ouro::window_state::minimized)
		dwAllFlags &=~ WS_MINIMIZE;

	// Transform the rectangle to with-border values for the new style
	oVB(AdjustWindowRect(&r, dwAllFlags, !!GetMenu(_hWnd)));

	// Update the flagging of the new style. This won't do anything without a call 
	// to SetWindowPos.
	SetLastError(0); // http://msdn.microsoft.com/en-us/library/ms644898(VS.85).aspx
	oWinSetTempChange(_hWnd, true);
	oVB(SetWindowLongPtr(_hWnd, GWL_STYLE, dwAllFlags));
	oWinSetTempChange(_hWnd, false);

	// Allow the system to calculate minimized/maximized sizes for us, so don't
	// modify them here, that will happen in SetWindowPlacement.
	if (!(Old.state == ouro::window_state::fullscreen && New.state == ouro::window_state::maximized))
	{
		UINT uFlags = SWP_NOZORDER|SWP_FRAMECHANGED;
		if (New.state == ouro::window_state::minimized || New.state == ouro::window_state::maximized)
			uFlags |= SWP_NOMOVE|SWP_NOSIZE;
		oVB(SetWindowPos(_hWnd, 0, r.left, r.top, oWinRectW(r), oWinRectH(r), uFlags));
	}

	// Now handle visibility, min/max/restore
	if (Old.state != New.state)
	{
		WINDOWPLACEMENT WP;
		WP.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(_hWnd, &WP);

		switch (New.state)
		{
			case ouro::window_state::invalid: case ouro::window_state::hidden:
				WP.showCmd = SW_HIDE;
				break;
			case ouro::window_state::minimized:
				WP.showCmd = SW_SHOWMINNOACTIVE;
				break;
			case ouro::window_state::maximized:
				WP.showCmd = SW_SHOWMAXIMIZED;
				break;
			case ouro::window_state::fullscreen:
			case ouro::window_state::restored:
				WP.showCmd = SW_RESTORE;
				WP.rcNormalPosition = r;
				break;
		}
		oVB(SetWindowPlacement(_hWnd, &WP));
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

ouro::display::id oWinGetDisplayId(HWND _hWnd)
{
	return ouro::display::get_id(MonitorFromWindow(_hWnd, MONITOR_DEFAULTTONEAREST));
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
	ouro::control_type::value type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case ouro::control_type::combotextbox:
			if (_SubItemIndex == ouro::invalid)
				oVB(SetWindowText(_hWnd, _Text));
			// pass through
		case ouro::control_type::combobox:
			if (_SubItemIndex != ouro::invalid)
			{
				oVERIFY(oWinControlDeleteSubItem(_hWnd, _Text, _SubItemIndex));
				oVERIFY(oWinControlInsertSubItem(_hWnd, _Text, _SubItemIndex));
			}
			break;

		case ouro::control_type::tab:
		{
			TCITEM tci;
			memset(&tci, 0, sizeof(tci));
			tci.dwStateMask = TCIF_TEXT;
			tci.pszText = (LPSTR)_Text;
			oVB(TabCtrl_SetItem(_hWnd, _SubItemIndex, &tci));
			break;
		}

		case ouro::control_type::floatbox:
		case ouro::control_type::floatbox_spinner:
			throw std::invalid_argument("For FloatBoxes and FloatBoxSpinners, use oWinControlSetValue instead.");

		default: 
			oVB(SetWindowText(_hWnd, _Text));
	}

	return true;
}

size_t oWinGetTruncatedLength(HWND _hWnd, const char* _StrSource)
{
	xxlstring temp(_StrSource);

	RECT rClient;
	GetClientRect(_hWnd, &rClient);
	oGDIScopedGetDC hDC(_hWnd);

	if (!PathCompactPathA(hDC, temp, oWinRectW(rClient) + 70)) // @tony: This constant was measured empirically. I think this should work without the +70, but truncation happens too aggressively.
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer must be at least MAX_PATH (%u) chars big", MAX_PATH);
		return 0;
	}

	if (!strcmp(temp, _StrSource))
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
		uri_string Truncated;
		const char* pCopy = _StrSource + strlen(_StrSource) - TruncatedLength + 3;
		oASSERT(pCopy >= _StrSource, "");
		if (-1 == snprintf(_StrDestination, _SizeofStrDestination, "...%s", pCopy))
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

	strlcpy(_StrDestination, _Path, _SizeofStrDestination);
	RECT rClient;
	GetClientRect(_hWnd, &rClient);
	oGDIScopedGetDC hDC(_hWnd);

	if (!PathCompactPathA(hDC, _StrDestination, oWinRectW(rClient) + 70)) // @tony: This constant was measured empirically. I think this should work without the +70, but truncation happens too aggressively.
	{
		oErrorSetLast(std::errc::no_buffer_space, "Buffer must be at least MAX_PATH (%u) chars big", MAX_PATH);
		return nullptr;
	}

	return _StrDestination;
}

char* oWinGetText(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, int _SubItemIndex)
{
	oWINVP(_hWnd);
	ouro::control_type::value type = ouro::control_type::unknown;
	if (_SubItemIndex != ouro::invalid)
		type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox:
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

		case ouro::control_type::tab:
		{
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)_StrDestination;
			item.cchTextMax = as_int(_SizeofStrDestination);
			oVB(TabCtrl_GetItem(_hWnd, _SubItemIndex, &item));
			return _StrDestination;
		}

		default:
			oVB(GetWindowText(_hWnd, _StrDestination, as_int(_SizeofStrDestination)));
			return _StrDestination;
	}
}

HICON oWinGetIcon(HWND _hWnd, bool _BigIcon)
{
	//oWINVP(_hWnd);
	ouro::control_type::value type = oWinControlGetType(_hWnd);
	switch (type)
	{
		case ouro::control_type::icon: return (HICON)SendMessage(_hWnd, STM_GETICON, 0, 0);
		default: return (HICON)SendMessage(_hWnd, WM_GETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), 0);
	}
}

bool oWinSetIconAsync(HWND _hWnd, HICON _hIcon, bool _BigIcon)
{
	ouro::control_type::value type = oWinControlGetType(_hWnd);
	if (oWinIsWindowThread(_hWnd))
	{
		switch (type)
		{
			case ouro::control_type::icon: SendMessage(_hWnd, STM_SETICON, (WPARAM)_hIcon, 0); break;
			default: SendMessage(_hWnd, WM_SETICON, (WPARAM)(_BigIcon ? ICON_BIG : ICON_SMALL), (LPARAM)_hIcon); break;
		}

		return true;
	}
	
	else
	{
		switch (type)
		{
			case ouro::control_type::icon: PostMessage(_hWnd, STM_SETICON, (WPARAM)_hIcon, 0); break;
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
				mstring text;
				GetWindowText(_hControl, text, (int)text.capacity());
				float f = 0.0f;
				atof(text, &f);
				snprintf(text, oSubclassFloatBoxFormat, f);
				SetWindowText(_hControl, text);
				break;
			}

			case WM_SETTEXT:
			{
				float f = 0.0f;
				mstring text = (const wchar_t*)_lParam; // handle wchar
				if (!atof(text, &f))
					return FALSE;
				
				// Ensure consistent formatting
				snprintf(text, oSubclassFloatBoxFormat, f);
				mwstring wtext = text;
				DefSubclassProc(_hControl, _uMsg, _wParam, (LPARAM)wtext.c_str());
				return FALSE;
			}

			case WM_GETDLGCODE:
			{
				// probably copy/paste, so let that through
				if ((GetKeyState(VK_LCONTROL) & 0x1000) || (GetKeyState(VK_RCONTROL) & 0x1000))
					return DLGC_WANTALLKEYS;

				mstring text;
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

				DWORD dwStart = ouro::invalid, dwEnd = ouro::invalid;
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

	return DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
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

	return DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
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

	return DefSubclassProc(_hControl, _uMsg, _wParam, _lParam);
}

static bool OnCreateAddSubItems(HWND _hControl, const ouro::control_info& _Desc)
{
	if (strchr(_Desc.text, '|'))
	{
		oVERIFY_R(oWinControlAddSubItems(_hControl, _Desc.text));
		oVERIFY_R(oWinControlSelectSubItem(_hControl, 0));
	}

	return true;
}

static bool OnCreateTab(HWND _hControl, const ouro::control_info& _Desc)
{
	oVERIFY_R(SetWindowSubclass(_hControl, oSubclassProcTab, oSubclassTabID, (DWORD_PTR)nullptr));
	return OnCreateAddSubItems(_hControl, _Desc);
}

static bool OnCreateGroup(HWND _hControl, const ouro::control_info& _Desc)
{
	return !!SetWindowSubclass(_hControl, oSubclassProcGroup, oSubclassGroupID, (DWORD_PTR)nullptr);
}

static bool OnCreateFloatBox(HWND _hControl, const ouro::control_info& _Desc)
{
	oVERIFY_R(SetWindowSubclass(_hControl, oSubclassProcFloatBox, oSubclassFloatBoxID, (DWORD_PTR)nullptr));
	SetWindowText(_hControl, _Desc.text);
	return true;
}

static bool OnCreateIcon(HWND _hControl, const ouro::control_info& _Desc)
{
	oWinControlSetIcon(_hControl, (HICON)_Desc.text);
	return true;
}

static bool OnCreateFloatBoxSpinner(HWND _hControl, const ouro::control_info& _Desc)
{
	ouro::control_info ActualFloatBoxDesc = _Desc;
	ActualFloatBoxDesc.type = ouro::control_type::floatbox;
	HWND hActualFloatBox = oWinControlCreate(ActualFloatBoxDesc);
	oVB(hActualFloatBox);
	SendMessage(_hControl, UDM_SETBUDDY, (WPARAM)hActualFloatBox, 0);
	SendMessage(_hControl, UDM_SETRANGE32, (WPARAM)std::numeric_limits<int>::lowest(), (LPARAM)std::numeric_limits<int>::max());
	return true;
}

static bool OnCreateMarquee(HWND _hControl, const ouro::control_info& _Desc)
{
	SendMessage(_hControl, PBM_SETMARQUEE, 1, 0);
	return true;
}

static int2 GetSizeExplicit(const ouro::control_info& _Desc)
{
	return _Desc.size;
}

static int2 GetSizeButtonDefault(const ouro::control_info& _Desc)
{
	return ouro::resolve_rect_size(_Desc.size, int2(75, 23));
}

static int2 GetSizeIcon(const ouro::control_info& _Desc)
{
	int2 iconSize = oGDIGetIconSize((HICON)_Desc.text);
	return ouro::resolve_rect_size(_Desc.size, iconSize);
}

struct CONTROL_CREATION_DESC
{
	const char* ClassName;
	DWORD dwStyle;
	DWORD dwExStyle;
	bool SetText;
	bool (*FinishCreation)(HWND _hControl, const ouro::control_info& _Desc);
	int2 (*GetSize)(const ouro::control_info& _Desc);
};

static const CONTROL_CREATION_DESC& oWinControlGetCreationDesc(ouro::control_type::value _Type)
{
	// === IF ADDING A NEW TYPE, MAKE SURE YOU ADD AN ENTRY TO oWinControlGetType ===

	static const CONTROL_CREATION_DESC sDescs[] = 
	{
		{ "Unknown", 0, 0, false, nullptr, GetSizeExplicit, },
		{ WC_BUTTON, WS_VISIBLE|WS_CHILD|BS_GROUPBOX, 0, true, OnCreateGroup, GetSizeExplicit, },
		{ WC_BUTTON, WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_PUSHBUTTON, 0, true, nullptr, GetSizeButtonDefault, },
		{ WC_BUTTON, WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_TOP|BS_AUTOCHECKBOX, 0, true, nullptr, GetSizeExplicit, },
		{ WC_BUTTON, WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_MULTILINE|BS_TOP|BS_AUTORADIOBUTTON, 0, true, nullptr, GetSizeExplicit, },
		{ WC_STATIC, WS_VISIBLE|WS_CHILD|SS_EDITCONTROL, 0, true, nullptr, GetSizeExplicit, },
		{ WC_STATIC, WS_VISIBLE|WS_CHILD|SS_EDITCONTROL|SS_CENTER, 0, true, nullptr, GetSizeExplicit, },
		{ "SysLink", WS_VISIBLE|WS_CHILD|WS_TABSTOP|LWS_NOPREFIX, 0, true, nullptr, GetSizeExplicit, },
		{ WC_EDIT, WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_READONLY|ES_NOHIDESEL|ES_AUTOHSCROLL, 0, true, nullptr, GetSizeExplicit, },
		{ WC_STATIC, WS_VISIBLE|WS_CHILD|SS_ICON|SS_REALSIZECONTROL, 0, false, OnCreateIcon, GetSizeIcon, },
		{ WC_EDIT, WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN, WS_EX_CLIENTEDGE, 0, nullptr, GetSizeExplicit, },
		{ WC_EDIT, WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_VSCROLL|ES_MULTILINE|ES_WANTRETURN, WS_EX_CLIENTEDGE, 0, nullptr, GetSizeExplicit, },
		{ WC_EDIT, WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_NOHIDESEL|ES_AUTOHSCROLL|ES_RIGHT, WS_EX_CLIENTEDGE, false, OnCreateFloatBox, GetSizeExplicit, },
		{ UPDOWN_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|UDS_ALIGNRIGHT|UDS_ARROWKEYS|UDS_HOTTRACK|UDS_NOTHOUSANDS|UDS_WRAP, 0,false, OnCreateFloatBoxSpinner, GetSizeExplicit, },
		{ WC_COMBOBOX, WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_HASSTRINGS, 0, false, OnCreateAddSubItems, GetSizeExplicit, },
		{ WC_COMBOBOX, WS_VISIBLE|WS_CHILD|WS_TABSTOP|CBS_DROPDOWN|CBS_HASSTRINGS, 0, false, OnCreateAddSubItems, GetSizeExplicit, },
		{ PROGRESS_CLASS, WS_VISIBLE|WS_CHILD, 0, true, nullptr, GetSizeExplicit, },
		{ PROGRESS_CLASS, WS_VISIBLE|WS_CHILD|PBS_MARQUEE, 0, true, OnCreateMarquee, GetSizeExplicit, },
		{ WC_TABCONTROL, WS_VISIBLE|WS_CHILD|WS_TABSTOP, WS_EX_CONTROLPARENT, true, OnCreateTab, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|TBS_NOTICKS, 0, true, nullptr, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP|TBS_NOTICKS|TBS_ENABLESELRANGE, 0, true, nullptr, GetSizeExplicit, },
		{ TRACKBAR_CLASS, WS_VISIBLE|WS_CHILD|WS_TABSTOP, 0, true, nullptr, GetSizeExplicit, }, 
		{ WC_LISTBOX, WS_VISIBLE|WS_CHILD|WS_TABSTOP|WS_VSCROLL|WS_HSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_HASSTRINGS|LBS_USETABSTOPS|WS_BORDER, 0, true, OnCreateAddSubItems, GetSizeExplicit, }, 
	};
	static_assert(ouro::control_type::count == oCOUNTOF(sDescs), "ouro::control_type::count count mismatch");
	return sDescs[_Type];
}

HWND oWinControlCreate(const ouro::control_info& _Desc)
{
	if (!_Desc.parent || _Desc.type == ouro::control_type::unknown)
		return (HWND)oErrorSetLast(std::errc::invalid_argument);
	oWINVP((HWND)_Desc.parent);

	const CONTROL_CREATION_DESC& CCDesc = oWinControlGetCreationDesc(_Desc.type);
	int2 size = CCDesc.GetSize(_Desc);

	HWND hWnd = CreateWindowEx(
		CCDesc.dwExStyle
		, CCDesc.ClassName
		, (CCDesc.SetText && _Desc.text) ? _Desc.text : ""
		, CCDesc.dwStyle | (_Desc.starts_new_group ? WS_GROUP : 0)
		, _Desc.position.x
		, _Desc.position.y
		, size.x
		, size.y
		, (HWND)_Desc.parent
		, (HMENU)_Desc.id
		, nullptr
		, nullptr);

	oVB(hWnd);

	oWinControlSetFont(hWnd, (HFONT)_Desc.font);
	if (CCDesc.FinishCreation)
	{
		if (!CCDesc.FinishCreation(hWnd, _Desc))
		{
			DestroyWindow(hWnd);
			hWnd = nullptr;
		}
	}

	return hWnd;
}

bool oWinControlDefaultOnNotify(HWND _hControl, const NMHDR& _NotifyMessageHeader, LRESULT* _plResult, ouro::control_type::value _Type)
{
	oWIN_CHECK(_hControl);
	bool ShortCircuit = false;
	if (_Type == ouro::control_type::unknown)
		_Type = oWinControlGetType(_NotifyMessageHeader.hwndFrom);
	switch (_Type)
	{
		case ouro::control_type::floatbox_spinner:
			if (_NotifyMessageHeader.code == UDN_DELTAPOS)
			{
				HWND hBuddyFloatBox = oWinControlGetBuddy(_NotifyMessageHeader.hwndFrom);
				if (hBuddyFloatBox)
				{
					const NMUPDOWN& ud = (const NMUPDOWN&)_NotifyMessageHeader;
					float f = oWinControlGetFloat(hBuddyFloatBox);
					oWinControlSetValue(hBuddyFloatBox, f + 0.01f * ud.iDelta);
				}
			}
			ShortCircuit = true;
			break;

		case ouro::control_type::hyperlabel:
		{
			if (_NotifyMessageHeader.code == NM_CLICK || _NotifyMessageHeader.code == NM_RETURN)
			{
				const NMLINK& NMLink = (const NMLINK&)_NotifyMessageHeader;
				xlstring asMultiByte(NMLink.item.szUrl);
				ouro::system::spawn_associated_application(asMultiByte);
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

bool oWinControlIsVisible(HWND _hControl)
{
	return !!(GetWindowLongPtr(_hControl, GWL_STYLE) & WS_VISIBLE);
}

bool oWinControlSetVisible(HWND _hControl, bool _Visible)
{
	if (!ShowWindow(_hControl, _Visible ? SW_SHOWNA : SW_HIDE))
		oVB(RedrawWindow(_hControl, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW));
	return true;
}

int oWinControlGetNumSubItems(HWND _hControl)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox: return ComboBox_GetCount(_hControl);
		case ouro::control_type::listbox: return ListBox_GetCount(_hControl);
		case ouro::control_type::tab: return TabCtrl_GetItemCount(_hControl);
		default: return 0;
	}
}

bool oWinControlClearSubItems(HWND _hControl)
{
	oWIN_CHECK(_hControl);
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox:
			ComboBox_ResetContent(_hControl);
			return true;
		case ouro::control_type::listbox:
			ListBox_ResetContent(_hControl);
			return true;
		case ouro::control_type::tab:
			oVB(TabCtrl_DeleteAllItems(_hControl));
			return true;
		default:
				break;
	}

	return true;
}

int oWinControlInsertSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex)
{
	oWIN_CHECK(_hControl);
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox:
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

		case ouro::control_type::listbox:
		{
			int index = LB_ERR;
			if (_SubItemIndex >= 0 && _SubItemIndex < ListBox_GetCount(_hControl))
				index = ListBox_InsertString(_hControl, _SubItemIndex, _SubItemText);
			else
				index = ListBox_AddString(_hControl, _SubItemText);

			if (index == LB_ERRSPACE)
			{
				oErrorSetLast(std::errc::no_buffer_space, "String is too large");
				index = CB_ERR;
			}

			return index;
		}

		case ouro::control_type::tab:
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
	return ouro::invalid;
}

bool oWinControlDeleteSubItem(HWND _hControl, const char* _SubItemText, int _SubItemIndex)
{
	oWIN_CHECK(_hControl);
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox:
			if (_SubItemIndex >= 0)
				ComboBox_DeleteString(_hControl, _SubItemIndex);
			break;

		case ouro::control_type::listbox:
			if (_SubItemIndex >= 0)
				ListBox_DeleteString(_hControl, _SubItemIndex);
			break;

		case ouro::control_type::tab:
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
	std::string copy(_DelimitedString);
	const char* tok = strtok_r((char*)copy.c_str(), delim, &ctx);
	while (tok)
	{
		oWinControlInsertSubItem(_hControl, tok, ouro::invalid);
		tok = strtok_r(nullptr, delim, &ctx);
	}
	return true;
}

int oWinControlFindSubItem(HWND _hControl, const char* _SubItemText)
{
	int index = CB_ERR;
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox:
		{
			int index = ComboBox_FindStringExact(_hControl, 0, _SubItemText);
			if (index == CB_ERR)
				oErrorSetLast(std::errc::invalid_argument, "Text %s was not found in %s %p (%d)", oSAFESTRN(_SubItemText), as_string(type), _hControl, GetDlgCtrlID(_hControl));
			break;
		}

		case ouro::control_type::listbox:
		{
			int index = ListBox_FindStringExact(_hControl, 0, _SubItemText);
			if (index == CB_ERR)
				oErrorSetLast(std::errc::invalid_argument, "Text %s was not found in %s %p (%d)", oSAFESTRN(_SubItemText), as_string(type), _hControl, GetDlgCtrlID(_hControl));
			break;
		}

		case ouro::control_type::tab:
		{
			mstring text;
			TCITEM item;
			item.mask = TCIF_TEXT;
			item.pszText = (LPSTR)text.c_str();
			item.cchTextMax = static_cast<int>(text.capacity());
			const int kCount = TabCtrl_GetItemCount(_hControl);
			for (int i = 0; i < kCount; i++)
			{
				if (TabCtrl_GetItem(_hControl, i, &item))
				{
					if (!strcmp(text, _SubItemText))
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox:
			if (CB_ERR == ComboBox_SetCurSel(_hControl, _SubItemIndex))
				return oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);
			break;

		case ouro::control_type::listbox:
			if (CB_ERR == ListBox_SetCurSel(_hControl, _SubItemIndex))
				return oErrorSetLastBadIndex(_hControl, type, _SubItemIndex);
			break;

		case ouro::control_type::tab:
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	int next = -1, count = 0;
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox:
			count = ComboBox_GetCount(_hControl);
			next = ComboBox_GetCurSel(_hControl) + _Offset;
			while (next < 0) next += count;
			while (next >= count) next -= count;
			if (CB_ERR == ComboBox_SetCurSel(_hControl, next))
				return oErrorSetLastBadIndex(_hControl, type, next);
			break;

		case ouro::control_type::listbox:
			count = ListBox_GetCount(_hControl);
			next = ListBox_GetCurSel(_hControl) + _Offset;
			while (next < 0) next += count;
			while (next >= count) next -= count;
			if (CB_ERR == ListBox_SetCurSel(_hControl, next))
				return oErrorSetLastBadIndex(_hControl, type, next);
			break;

		case ouro::control_type::tab:
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::combobox:
		case ouro::control_type::combotextbox: return ComboBox_GetCurSel(_hControl);
		case ouro::control_type::listbox: return ListBox_GetCurSel(_hControl);
		case ouro::control_type::tab: return TabCtrl_GetCurSel(_hControl);
		default: break;
	}
	return ouro::invalid;
}

ouro::control_type::value oWinControlGetType(HWND _hControl)
{
	mstring ClassName;
	if (!GetClassName(_hControl, ClassName, static_cast<int>(ClassName.capacity())))
		return ouro::control_type::unknown;
	
	if (!_stricmp("Button", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hControl, GWL_STYLE);
		switch (dwStyle)
		{
			case BS_GROUPBOX: return ouro::control_type::group;
			case BS_PUSHBUTTON: 
			case BS_DEFPUSHBUTTON: return ouro::control_type::button;
			case BS_AUTOCHECKBOX: return ouro::control_type::checkbox;
			case BS_AUTORADIOBUTTON: return ouro::control_type::radio;
			default: return ouro::control_type::unknown;
		}
	}

	else if (!_stricmp("Static", ClassName))
	{
		LONG dwStyle = 0xff & GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle & SS_ICON)
			return ouro::control_type::icon;
		else if (dwStyle & SS_CENTER)
			return ouro::control_type::label_centered;
		else if (dwStyle & SS_SIMPLE)
			return ouro::control_type::label;
		else
			return ouro::control_type::unknown;
	}

	else if (!_stricmp("Edit", ClassName))
	{
		DWORD dwStyle = 0xffff & (DWORD)GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle == (dwStyle & oWinControlGetCreationDesc(ouro::control_type::label_selectable).dwStyle))
			return ouro::control_type::label_selectable;

			DWORD_PTR data;
		if (GetWindowSubclass(_hControl, oSubclassProcFloatBox, oSubclassFloatBoxID, &data))
			return ouro::control_type::floatbox;

		else if (dwStyle == (dwStyle & oWinControlGetCreationDesc(ouro::control_type::textbox).dwStyle))
			return ouro::control_type::textbox;

		else
			return ouro::control_type::unknown;
	}

	else if (!_stricmp("ComboBox", ClassName))
	{
		LONG dwStyle = 0xf & GetWindowLong(_hControl, GWL_STYLE);
		switch (dwStyle)
		{
			case CBS_DROPDOWNLIST: return ouro::control_type::combobox;
			case CBS_DROPDOWN: return ouro::control_type::combotextbox;
			default: return ouro::control_type::unknown;
		}
	}

	else if (!_stricmp(oWinControlGetCreationDesc(ouro::control_type::floatbox_spinner).ClassName, ClassName))
		return ouro::control_type::floatbox_spinner;

	else if (!_stricmp(oWinControlGetCreationDesc(ouro::control_type::hyperlabel).ClassName, ClassName))
		return ouro::control_type::hyperlabel;

	else if (!_stricmp(oWinControlGetCreationDesc(ouro::control_type::tab).ClassName, ClassName))
	{
		// Don't identify a generic tab as an oGUI tab, only ones that
		// behave properly
		DWORD_PTR data;
		if (GetWindowSubclass(_hControl, oSubclassProcTab, oSubclassTabID, &data))
			return ouro::control_type::tab;
		return ouro::control_type::unknown;
	}
	else if (!_stricmp(oWinControlGetCreationDesc(ouro::control_type::progressbar).ClassName, ClassName))
	{
		DWORD dwStyle = (DWORD)GetWindowLong(_hControl, GWL_STYLE);
		if (dwStyle & PBS_MARQUEE)
			return ouro::control_type::progressbar_unknown;
		return ouro::control_type::progressbar;
	}
	else if (!_stricmp(oWinControlGetCreationDesc(ouro::control_type::slider).ClassName, ClassName))
	{
		DWORD dwStyle = (DWORD)GetWindowLong(_hControl, GWL_STYLE);

		if (dwStyle & TBS_ENABLESELRANGE)
			return ouro::control_type::slider_selectable;
		else if (dwStyle & TBS_NOTICKS)
			return ouro::control_type::slider;
		else
			return ouro::control_type::slider_with_ticks;
	}
	else if (!_stricmp(oWinControlGetCreationDesc(ouro::control_type::listbox).ClassName, ClassName))
		return ouro::control_type::listbox;

	return ouro::control_type::unknown;
}

int2 oWinControlGetInitialSize(ouro::control_type::value _Type, const int2& _Size)
{
	ouro::control_info d;
	d.type = _Type;
	d.size = _Size;
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::floatbox_spinner:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass through
		case ouro::control_type::textbox:
		case ouro::control_type::textbox_scrollable:
		case ouro::control_type::combotextbox:
		case ouro::control_type::floatbox:
		{
			uint start = 0, end = 0;
			SendMessage(_hControl, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
			size_t len = end - start;
			if (len >= _SizeofStrDestination)
			{
				oErrorSetLast(std::errc::no_buffer_space, "Buffer too small to receive string");
				return nullptr;
			}

			oVB(GetWindowText(_hControl, _StrDestination, as_int(_SizeofStrDestination)));

			if (start != end)
			{
				if (start)
					strlcpy(_StrDestination, _StrDestination + start, _SizeofStrDestination);
				_StrDestination[len] = 0;
			}

			return _StrDestination;
		}

		case ouro::control_type::listbox:
		{
			int index = ListBox_GetCurSel(_hControl);
			if (index != LB_ERR)
				return nullptr;

			size_t len = ListBox_GetTextLen(_hControl, index);
			if (len >= _SizeofStrDestination)
			{
				oErrorSetLast(std::errc::no_buffer_space, "Buffer too small to receive string");
				return nullptr;
			}

			if (LB_ERR == ListBox_GetText(_hControl, index, _StrDestination))
			{
				oErrorSetLast(std::errc::protocol_error, "GetText failed");
				return nullptr;
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::floatbox_spinner:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass thru
		case ouro::control_type::floatbox:
		case ouro::control_type::textbox:
		case ouro::control_type::textbox_scrollable:
		case ouro::control_type::combotextbox:
		case ouro::control_type::label_selectable:
			Edit_SetSel(_hControl, _Start, _Start+_Length);
			return true;
		case ouro::control_type::slider_selectable:
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
	mstring text;
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::floatbox_spinner:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass through
		default:
			snprintf(text, oSubclassFloatBoxFormat, _Value);
			oVB(SetWindowText(_hControl, text));
			return true;
	}
}

float oWinControlGetFloat(HWND _hControl)
{
	mstring text;
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::floatbox_spinner:
			_hControl = oWinControlGetBuddy(_hControl);
			// pass through
		default: 
			GetWindowText(_hControl, text.c_str(), (int)text.capacity());
			break;
	}

	float f = 0.0f;
	if (!atof(text, &f))
		return std::numeric_limits<float>::quiet_NaN();
	return f;
}

bool oWinControlSetIcon(HWND _hControl, HICON _hIcon, int _SubItemIndex)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::icon:
			if (_SubItemIndex != ouro::invalid)
				return oErrorSetLast(std::errc::invalid_argument, "Invalid _SubItemIndex");
			SendMessage(_hControl, STM_SETICON, (WPARAM)_hIcon, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

HICON oWinControlGetIcon(HWND _hControl, int _SubItemIndex)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::icon:
			if (_SubItemIndex != ouro::invalid)
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::checkbox:
		case ouro::control_type::radio:
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::checkbox:
		case ouro::control_type::radio:
			Button_SetCheck(_hControl, _Checked ? BST_CHECKED : BST_UNCHECKED);
	return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetRange(HWND _hControl, int _Min, int _Max)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::slider:
		case ouro::control_type::slider_with_ticks:
		case ouro::control_type::slider_selectable:
			SendMessage(_hControl, TBM_SETRANGEMIN, TRUE, _Min);
			SendMessage(_hControl, TBM_SETRANGEMAX, TRUE, _Max);
			return true;
		case ouro::control_type::progressbar:
			SendMessage(_hControl, PBM_SETRANGE32, _Min, _Max);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlGetRange(HWND _hControl, int* _pMin, int* _pMax)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::slider:
		case ouro::control_type::slider_with_ticks:
		case ouro::control_type::slider_selectable:
			if (_pMin) *_pMin = (int)SendMessage(_hControl, TBM_GETRANGEMIN, 0, 0);
			if (_pMax) *_pMax = (int)SendMessage(_hControl, TBM_GETRANGEMAX, 0, 0);
			return true;
		case ouro::control_type::progressbar:
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
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::slider:
		case ouro::control_type::slider_with_ticks:
		case ouro::control_type::slider_selectable:
			SendMessage(_hControl, _bNotify ? TBM_SETPOSNOTIFY : TBM_SETPOS, 0, _Position);
			return true;
		case ouro::control_type::progressbar:
			SendMessage(_hControl, PBM_SETPOS, _Position, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

int oWinControlGetRangePosition(HWND _hControl)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::slider:
		case ouro::control_type::slider_with_ticks:
		case ouro::control_type::slider_selectable: return (int)SendMessage(_hControl, TBM_GETPOS, 0, 0);
		case ouro::control_type::progressbar: return (int)SendMessage(_hControl, PBM_GETPOS, 0, 0);
		default: return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetTick(HWND _hControl, int _Position)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch(type)
	{
	case ouro::control_type::slider_with_ticks:
		SendMessage(_hControl, TBM_SETTIC, 0, _Position);
		return true;
	default:
		return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlClearTicks(HWND _hControl)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch(type)
	{
	case ouro::control_type::slider_with_ticks:
		SendMessage(_hControl, TBM_CLEARTICS, 1, 0);
		return true;
	default:
		return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlSetErrorState(HWND _hControl, bool _InErrorState)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::progressbar:
			SendMessage(_hControl, PBM_SETSTATE, _InErrorState ? PBST_ERROR : PBST_NORMAL, 0);
			return true;
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlGetErrorState(HWND _hControl)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::progressbar:
			oErrorSetLast(0);
			return PBST_ERROR == SendMessage(_hControl, PBM_GETSTATE, 0, 0);
		default:
			return oErrorSetLastBadType(_hControl, type);
	}
}

bool oWinControlClampPositionToSelected(HWND _hControl)
{
	ouro::control_type::value type = oWinControlGetType(_hControl);
	switch (type)
	{
		case ouro::control_type::slider_selectable:
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

void oMoveMouseCursorOffscreen()
{
	int2 p, sz;
	ouro::display::virtual_rect(&p.x, &p.y, &sz.x, &sz.y);
	SetCursorPos(p.x + sz.x, p.y-1);
}
