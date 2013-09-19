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
#include <oPlatform/oTest.h>
#include <oPlatform/oWindow.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinControlSet.h>
#include <oPlatform/Windows/oGDI.h>

static const bool kInteractiveMode = false;

class oSystemProperties
{
public:
	enum CONTROL
	{
		ID_TAB,

		ID_COMPUTER_ICON,
		ID_COMPUTER_FIRST = ID_COMPUTER_ICON,
		ID_COMPUTER_LABEL,
		ID_COMPUTER_DESC_LABEL,
		ID_COMPUTER_DESC_TEXTBOX,
		ID_COMPUTER_DESC_EXAMPLE,
		ID_COMPUTER_NAME_LABEL,
		ID_COMPUTER_NAME_VALUE,
		ID_COMPUTER_WORKGROUP_LABEL,
		ID_COMPUTER_WORKGROUP_VALUE,
		ID_COMPUTER_NETWORK_ID_LABEL,
		ID_COMPUTER_NETWORK_ID_BUTTON,
		ID_COMPUTER_WORKGROUP_RENAME_LABEL,
		ID_COMPUTER_WORKGROUP_RENAME_BUTTON,
		ID_COMPUTER_LAST = ID_COMPUTER_WORKGROUP_RENAME_BUTTON,

		ID_HARDWARE_DEVICE_MANAGER_GROUP,
		ID_HARDWARE_FIRST = ID_HARDWARE_DEVICE_MANAGER_GROUP,
		ID_HARDWARE_DEVICE_MANAGER_ICON,
		ID_HARDWARE_DEVICE_MANAGER_LABEL,
		ID_HARDWARE_DEVICE_MANAGER_BUTTON,
		ID_HARDWARE_DEVICE_INSTALL_GROUP,
		ID_HARDWARE_DEVICE_INSTALL_ICON,
		ID_HARDWARE_DEVICE_INSTALL_LABEL,
		ID_HARDWARE_DEVICE_INSTALL_BUTTON,
		ID_HARDWARE_LAST = ID_HARDWARE_DEVICE_INSTALL_BUTTON,

		ID_ADVANCED_ADMIN_LOGON,
		ID_ADVANCED_FIRST = ID_ADVANCED_ADMIN_LOGON,
		ID_ADVANCED_PERFORMANCE_GROUP,
		ID_ADVANCED_PERFORMANCE_LABEL,
		ID_ADVANCED_PERFORMANCE_SETTINGS,
		ID_ADVANCED_USER_PROFILES_GROUP,
		ID_ADVANCED_USER_PROFILES_LABEL,
		ID_ADVANCED_USER_PROFILES_SETTINGS,
		ID_ADVANCED_STARTUP_GROUP,
		ID_ADVANCED_STARTUP_LABEL,
		ID_ADVANCED_STARTUP_SETTINGS,
		ID_ADVANCED_ENVIRONMENT_VARIABLES,
		ID_ADVANCED_LAST = ID_ADVANCED_ENVIRONMENT_VARIABLES,

		ID_SYSPROT_ICON,
		ID_SYSPROT_FIRST = ID_SYSPROT_ICON,
		ID_SYSPROT_LABEL,
		ID_SYSPROT_RESTORE_TITLE,
		ID_SYSPROT_RESTORE_LABEL,
		ID_SYSPROT_RESTORE_BUTTON,
		ID_SYSPROT_SETTINGS_TITLE,
		ID_SYSPROT_LISTBOX,
		ID_SYSPROT_CONFIG_RESTORE_LABEL,
		ID_SYSPROT_CONFIG_RESTORE_BUTTON,
		ID_SYSPROT_RESTORE_POINT_LABEL,
		ID_SYSPROT_RESTORE_POINT_BUTTON,
		ID_SYSPROT_LAST = ID_SYSPROT_RESTORE_POINT_BUTTON,

		ID_REMOTE_ASSISTANCE_GROUP,
		ID_REMOTE_FIRST = ID_REMOTE_ASSISTANCE_GROUP,
		ID_ALLOW_REMOTE_ASSISTANCE,
		ID_REMOTE_ASSISTANCE_WHAT_HAPPENS,
		ID_REMOTE_ADVANCED,
		ID_REMOTE_DESKTOP_GROUP,
		ID_REMOTE_DESKTOP_CLICK,
		ID_REMOTE_DESKTOP_RADIO1,
		ID_REMOTE_DESKTOP_RADIO2,
		ID_REMOTE_DESKTOP_RADIO3,
		ID_REMOTE_DESKTOP_HELP_ME_CHOOSE,
		ID_REMOTE_DESKTOP_SELECT_USERS,
		ID_REMOTE_LAST = ID_REMOTE_DESKTOP_SELECT_USERS,

		ID_OK,
		ID_CANCEL,
		ID_APPLY,

		NUM_CONTROLS,

		ID_RELOAD_UI,
	};

public:
	oSystemProperties(bool* _pSuccess);

	oWindow* GetWindow() { return Window; }
	bool GetRunning() const { return Running; }

	void ShowTab(int _TabIndex)
	{
		oASSERT(Window->IsWindowThread(), "wrong thread");
		oWinControlSelectSubItem(ControlSet[ID_TAB], _TabIndex);
	}

private:
	oWinControlSet ControlSet;
	oStd::intrusive_ptr<oWindow> Window;
	bool Running;

	bool Reload(HWND _hParent, const int2& _ClientSize);

	void Show(int _First, int _Last, bool _Show);
	void Show(int _TabIndex, bool _Show)
	{
		switch (_TabIndex)
		{
			case 0: Show(ID_COMPUTER_FIRST, ID_COMPUTER_LAST, _Show); break;
			case 1: Show(ID_HARDWARE_FIRST, ID_HARDWARE_LAST, _Show); break;
			case 2: Show(ID_ADVANCED_FIRST, ID_ADVANCED_LAST, _Show); break;
			case 3: Show(ID_SYSPROT_FIRST, ID_SYSPROT_LAST, _Show); break;
			case 4: Show(ID_REMOTE_FIRST, ID_REMOTE_LAST, _Show); break;
			oNODEFAULT;
		}
	}

	void EventHook(const oGUI_EVENT_DESC& _Event);
	void ActionHook(const oGUI_ACTION_DESC& _Action);
};

namespace oStd {

bool from_string(oSystemProperties::CONTROL* _pControl, const char* _StrSource)
{
	static const char* sStrings[] = 
	{
		"ID_TAB",

		"ID_COMPUTER_ICON",
		"ID_COMPUTER_LABEL",
		"ID_COMPUTER_DESC_LABEL",
		"ID_COMPUTER_DESC_TEXTBOX",
		"ID_COMPUTER_DESC_EXAMPLE",
		"ID_COMPUTER_NAME_LABEL",
		"ID_COMPUTER_NAME_VALUE",
		"ID_COMPUTER_WORKGROUP_LABEL",
		"ID_COMPUTER_WORKGROUP_VALUE",
		"ID_COMPUTER_NETWORK_ID_LABEL",
		"ID_COMPUTER_NETWORK_ID_BUTTON",
		"ID_COMPUTER_WORKGROUP_RENAME_LABEL",
		"ID_COMPUTER_WORKGROUP_RENAME_BUTTON",

		"ID_HARDWARE_DEVICE_MANAGER_GROUP",
		"ID_HARDWARE_DEVICE_MANAGER_ICON",
		"ID_HARDWARE_DEVICE_MANAGER_LABEL",
		"ID_HARDWARE_DEVICE_MANAGER_BUTTON",
		"ID_HARDWARE_DEVICE_INSTALL_GROUP",
		"ID_HARDWARE_DEVICE_INSTALL_ICON",
		"ID_HARDWARE_DEVICE_INSTALL_LABEL",
		"ID_HARDWARE_DEVICE_INSTALL_BUTTON",

		"ID_ADVANCED_ADMIN_LOGON",
		"ID_ADVANCED_PERFORMANCE_GROUP",
		"ID_ADVANCED_PERFORMANCE_LABEL",
		"ID_ADVANCED_PERFORMANCE_SETTINGS",
		"ID_ADVANCED_USER_PROFILES_GROUP",
		"ID_ADVANCED_USER_PROFILES_LABEL",
		"ID_ADVANCED_USER_PROFILES_SETTINGS",
		"ID_ADVANCED_STARTUP_GROUP",
		"ID_ADVANCED_STARTUP_LABEL",
		"ID_ADVANCED_STARTUP_SETTINGS",
		"ID_ADVANCED_ENVIRONMENT_VARIABLES",

		"ID_SYSPROT_ICON",
		"ID_SYSPROT_LABEL",
		"ID_SYSPROT_RESTORE_TITLE",
		"ID_SYSPROT_RESTORE_LABEL",
		"ID_SYSPROT_RESTORE_BUTTON",
		"ID_SYSPROT_SETTINGS_TITLE",
		"ID_SYSPROT_LISTBOX",
		"ID_SYSPROT_CONFIG_RESTORE_LABEL",
		"ID_SYSPROT_CONFIG_RESTORE_BUTTON",
		"ID_SYSPROT_RESTORE_POINT_LABEL",
		"ID_SYSPROT_RESTORE_POINT_BUTTON",

		"ID_REMOTE_ASSISTANCE_GROUP",
		"ID_ALLOW_REMOTE_ASSISTANCE",
		"ID_REMOTE_ASSISTANCE_WHAT_HAPPENS",
		"ID_REMOTE_ADVANCED",
		"ID_REMOTE_DESKTOP_GROUP",
		"ID_REMOTE_DESKTOP_CLICK",
		"ID_REMOTE_DESKTOP_RADIO1",
		"ID_REMOTE_DESKTOP_RADIO2",
		"ID_REMOTE_DESKTOP_RADIO3",
		"ID_REMOTE_DESKTOP_HELP_ME_CHOOSE",
		"ID_REMOTE_DESKTOP_SELECT_USERS",

		"ID_OK",
		"ID_CANCEL",
		"ID_APPLY",
	};
	static_assert(oCOUNTOF(sStrings) == oSystemProperties::NUM_CONTROLS, "Mismatched Control enum");
	for (size_t i = 0; i < oCOUNTOF(sStrings); i++)
	{
		if (!strcmp(_StrSource, sStrings[i]))
		{
			*_pControl = (oSystemProperties::CONTROL)i;
			return true;
		}
	}

	return false;
}

} // namespace oStd

oSystemProperties::oSystemProperties(bool* _pSuccess)
	: Running(true)
{
	*_pSuccess = false;

	oWINDOW_INIT init;
	init.Title = "TESTWindowSysDialog";
	init.EventHook = oBIND(&oSystemProperties::EventHook, this, oBIND1);
	init.ActionHook = oBIND(&oSystemProperties::ActionHook, this, oBIND1);
	init.Shape.Style = oGUI_WINDOW_DIALOG;
	init.Shape.State = oGUI_WINDOW_RESTORED;
	init.Shape.ClientSize = int2(410,436);

	if (!oWindowCreate(init, &Window))
		return;

	{
		#if oENABLE_ASSERTS
			HWND hWnd = (HWND)Window->GetNativeHandle();
			RECT rClient;
			GetClientRect(hWnd, &rClient);
			oASSERT(oWinRectSize(rClient) == init.Shape.ClientSize, "Client size mismatch");
		#endif

		// Disable anti-aliasing since on Windows ClearType seems to be non-deterministic
		Window->Dispatch([&]
		{
			HWND hWnd = (HWND)Window->GetNativeHandle();

			oGUI_FONT_DESC fd;
			HFONT hCurrent = oWinGetFont(hWnd);
			oGDIGetFontDesc(hCurrent, &fd);
			fd.AntiAliased = false;
			HFONT hNew = oGDICreateFont(fd);
			oWinSetFont(hWnd, hNew);
		});
	}

	if (kInteractiveMode)
	{
		oGUI_HOTKEY_DESC_NO_CTOR HotKeys[] = 
		{
			{ oGUI_KEY_F3, ID_RELOAD_UI, false, false, false },
		};

		Window->SetHotKeys(HotKeys);
	}
	
	*_pSuccess = true;
}

bool oSystemProperties::Reload(HWND _hParent, const int2& _ClientSize)
{
	std::shared_ptr<oStd::xml> XML;

	#if 0
		// Load from files for fast iteration, then compile the xml using oFile2cpp
		XML = oXMLLoad("../../SDK/oPlatform/Tests/SystemProperties.xml");
	#else
		void GetDescSystemProperties_xml(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
		const char* pBufferName = nullptr;
		const void* pBuffer = nullptr;
		size_t Size = 0;
		GetDescSystemProperties_xml(&pBufferName, &pBuffer, &Size);
		XML = std::make_shared<oStd::xml>(pBufferName, (char*)pBuffer, nullptr);
	#endif

	ControlSet.Deinitialize();

	oWinControlSet::IDFROMSTRING FromString = [&](int* _pID, const char* _StrSource)->bool { return oStd::from_string((oSystemProperties::CONTROL*)_pID, _StrSource); };
	oVERIFY(ControlSet.Initialize(_hParent, _ClientSize, *XML, FromString));

	if (ControlSet[ID_COMPUTER_NAME_VALUE])
	{
		oStd::mstring Hostname;
		if (kInteractiveMode)
			oCore::system::host_name(Hostname);
		else
			Hostname = "My local hostname";
		oWinControlSetText(ControlSet[ID_COMPUTER_NAME_VALUE], Hostname);
	}

	if (ControlSet[ID_COMPUTER_WORKGROUP_VALUE])
	{
		oStd::mstring WorkgroupName;
		if (kInteractiveMode)
			oWinGetWorkgroupName(WorkgroupName, WorkgroupName.capacity());
		else
			WorkgroupName = "MY_WORKGROUP";
		oWinControlSetText(ControlSet[ID_COMPUTER_WORKGROUP_VALUE], WorkgroupName);
	}

	return true;
}

void oSystemProperties::EventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_CREATING:
		{
			oVERIFY(Reload((HWND)_Event.hWindow, _Event.AsCreate().Shape.ClientSize));
			break;
		}
	}
}

void oSystemProperties::ActionHook(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_CONTROL_ACTIVATED:
		{
			switch (_Action.DeviceID)
			{
				case ID_OK: Running = false; break;
				case ID_CANCEL: Running = false; break;
				case ID_APPLY: oWinEnable((HWND)_Action.hWindow, false); break;
				default: break;
			}

			if (_Action.DeviceID != ID_APPLY)
				oWinEnable((HWND)ControlSet[ID_APPLY], true);
			break;
		}

		case oGUI_ACTION_CONTROL_SELECTION_CHANGING:
		{
			int index = oWinControlGetSelectedSubItem((HWND)_Action.hWindow);
			Show(index, false);
			break;
		}

		case oGUI_ACTION_CONTROL_SELECTION_CHANGED:
		{
			int index = oWinControlGetSelectedSubItem((HWND)_Action.hWindow);
			Show(index, true);
			break;
		}

		case oGUI_ACTION_HOTKEY:
		{
			if (_Action.DeviceID == ID_RELOAD_UI)
				oVERIFY(Reload((HWND)Window->GetNativeHandle(), Window->GetClientSize()));
			
			break;
		}
	}
}

void oSystemProperties::Show(int _First, int _Last, bool _Show)
{
	for (int i = _First; i <= _Last; i++)
		oWinControlSetVisible(ControlSet[i], _Show);
}

static void OverwriteVariableColors(oImage* _pImage)
{
	static const int2 VarCoords[] = { int2(25,125), int2(26,125), int2(30,125), int2(31,125), int2(32,125), int2(83,125), int2(84,125), int2(135,125), int2(136,125), int2(137,125), int2(173,125), int2(174,125), int2(175,125), };
	oFOR(const int2& c, VarCoords)
		_pImage->Put(c, oStd::Red);
}

struct PLATFORM_WindowSysDialog : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		if (oCore::system::is_remote_session())
		{
			snprintf(_StrStatus, _SizeofStrStatus, "Detected remote session: differing text anti-aliasing will cause bad image compares");
			return SKIPPED;
		}

		bool success = false;
		oSystemProperties test(&success);
		oTESTB0(success);
		
		do
		{
			test.GetWindow()->FlushMessages();

		} while (kInteractiveMode && test.GetRunning());

		if (!kInteractiveMode)
		{
			oStd::future<oStd::intrusive_ptr<oImage>> snapshot = test.GetWindow()->CreateSnapshot();
			test.GetWindow()->FlushMessages();

			oTESTFI(snapshot);

			for (int i = 0; i < 5; i++)
			{
				test.ShowTab(i);
				snapshot = test.GetWindow()->CreateSnapshot();
				test.GetWindow()->FlushMessages();
				
				// special-case instance that returns 4 pixels off due to what seems to
				// be indeterminate behavior in win32 rendering (clear type) that cannot
				// be turned off dynamically.
				if (i == 3)
				{
					oTEST_FUTURE(snapshot);
					oStd::intrusive_ptr<oImage> image;
					try { image = snapshot.get(); }
					catch (std::exception& e)
					{
						oErrorSetLast(e);
						return oTest::FAILURE;
					}
					OverwriteVariableColors(image);
					oTESTI2(image, i);
				}

				else
					oTESTFI2(snapshot, i);
			}
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_WindowSysDialog);
