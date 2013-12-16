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
#include <oGUI/oGUIMenu.h>
#include <oGUI/oMsgBox.h>
#include <oBase/timer.h>
#include <oGUI/window.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/Windows/oWinCommonDialog.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/system.h>
#include <oCore/windows/win_error.h>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

static const bool kInteractiveMode = false;

class oWindowUITest
{
public:
	oWindowUITest(bool* _pSuccess);

	window* GetWindow() { return Window.get(); }
	bool GetRunning() const { return Running; }

private:
	std::shared_ptr<window> Window;
	bool Running;

	enum
	{
		MENU_FILE_OPEN,
		MENU_FILE_SAVE,
		MENU_FILE_SAVEAS,
		MENU_FILE_EXIT,
		MENU_EDIT_CUT,
		MENU_EDIT_COPY,
		MENU_EDIT_PASTE,
		MENU_EDIT_COLOR,
		MENU_EDIT_FONT,
		MENU_VIEW_SOLID,
		MENU_VIEW_WIREFRAME,
		MENU_VIEW_SHOW_STATUSBAR,
		MENU_HELP_ABOUT,
	};

	oGUI_MENU hFileMenu;
	oGUI_MENU hEditMenu;
	oGUI_MENU hViewMenu;
	oGUI_MENU hStatusBarMenu;
	oGUI_MENU hHelpMenu;

	void InitMenu(oGUI_MENU _hWindowMenu);
	void InitVCRControls(HWND _hParent, const int2& _Position);

	bool OnCreate(HWND _hWnd, oGUI_MENU _hMenu);
	void OnMenuCommand(HWND _hWnd, int _MenuID);

	enum
	{
		ID_PUSHME,
		ID_FLOATBOX,
		ID_FLOATBOX_SPINNER,
		ID_EASY,

		ID_BIG_BACK,
		ID_BACK,
		ID_PLAY_PAUSE,
		ID_FORWARD,
		ID_BIG_FORWARD,

		ID_SLIDER,
		ID_SLIDER_SELECTABLE,

		ID_PROGRESSBAR,
		ID_MARQUEE,

		ID_COMBOBOX,
		ID_COMBOTEXTBOX,

		NUM_CONTROLS,
	};

	HWND Controls[NUM_CONTROLS];
	oGUI_BORDER_STYLE BorderStyle;

	void EventHook(const oGUI_EVENT_DESC& _Event);
	void ActionHook(const oGUI_ACTION_DESC& _Action);
};

void oWindowUITest::EventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_SIZED:
			oTRACE("NewClientSize = %dx%d%s", _Event.AsShape().Shape.ClientSize.x, _Event.AsShape().Shape.ClientSize.y, _Event.AsShape().Shape.State == oGUI_WINDOW_MINIMIZED ? " (minimized)" : "");
			break;
		case oGUI_CREATING:
		{
			OnCreate((HWND)_Event.AsCreate().hWindow, _Event.AsCreate().hMenu);
			break;
		}

		default:
			break;
	}
}

void oWindowUITest::ActionHook(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_MENU:
		case oGUI_ACTION_HOTKEY:
			OnMenuCommand((HWND)_Action.hWindow, _Action.DeviceID);
			// pass through
		case oGUI_ACTION_CONTROL_ACTIVATED:
		{
			ouro::lstring text;
			oWinControlGetText(text, (HWND)_Action.hWindow);
			oTRACE("Action %s \"%s\" code=%d", ouro::as_string(oWinControlGetType((HWND)_Action.hWindow)), text.c_str(), _Action.ActionCode);
			break;
		}
		case oGUI_ACTION_KEY_DOWN:
		case oGUI_ACTION_KEY_UP:
			break;
		case oGUI_ACTION_POINTER_MOVE:
			if (kInteractiveMode)
				Window->set_status_text(0, "Cursor: %dx%d", (int)_Action.Position.x, (int)_Action.Position.y);
			break;
		default:
			break;
	}
}

void oWindowUITest::InitMenu(oGUI_MENU _hWindowMenu)
{
	hFileMenu = oGUIMenuCreate();
	hEditMenu = oGUIMenuCreate();
	hViewMenu = oGUIMenuCreate();
	hStatusBarMenu = oGUIMenuCreate();
	hHelpMenu = oGUIMenuCreate();

	oGUIMenuAppendSubmenu(_hWindowMenu, hFileMenu, "&File");
		oGUIMenuAppendItem(hFileMenu, MENU_FILE_OPEN, "&Open...");
		oGUIMenuAppendItem(hFileMenu, MENU_FILE_SAVE, "&Save");
		oGUIMenuAppendItem(hFileMenu, MENU_FILE_SAVEAS, "Save &As...");
		oGUIMenuAppendSeparator(hFileMenu);
		oGUIMenuAppendItem(hFileMenu, MENU_FILE_EXIT, "E&xit");
	oGUIMenuAppendSubmenu(_hWindowMenu, hEditMenu, "&Edit");
		oGUIMenuAppendItem(hEditMenu, MENU_EDIT_CUT, "Cu&t");
		oGUIMenuAppendItem(hEditMenu, MENU_EDIT_COPY, "&Copy");
		oGUIMenuAppendItem(hEditMenu, MENU_EDIT_PASTE, "&Paste");
		oGUIMenuAppendSeparator(hEditMenu);
		oGUIMenuAppendItem(hEditMenu, MENU_EDIT_COLOR, "Co&lor");
		oGUIMenuAppendItem(hEditMenu, MENU_EDIT_FONT, "&Font");
	oGUIMenuAppendSubmenu(_hWindowMenu, hViewMenu, "&View");
		oGUIMenuAppendItem(hViewMenu, MENU_VIEW_SOLID, "&Solid\tCtrl+S");
		oGUIMenuCheck(hViewMenu, MENU_VIEW_SOLID, true);
		oGUIMenuAppendItem(hViewMenu, MENU_VIEW_WIREFRAME, "&Wireframe\tCtrl+W");
		oGUIMenuAppendSeparator(hViewMenu);
		oGUIMenuAppendItem(hViewMenu, MENU_VIEW_SHOW_STATUSBAR, "&Show Statusbar");
		oGUIMenuCheck(hViewMenu, MENU_VIEW_SHOW_STATUSBAR, true);
	oGUIMenuAppendSubmenu(_hWindowMenu, hHelpMenu, "&Help");
		oGUIMenuAppendItem(hHelpMenu, MENU_HELP_ABOUT, "&About...");
}

void oWindowUITest::InitVCRControls(HWND _hParent, const int2& _Position)
{
	static const int2 kButtonSize = int2(25,25);
	static const int2 kButtonSpacing = int2(27,0);

	oGUI_FONT_DESC fd;
	fd.FontName = "Webdings";
	fd.PointSize = 12;
	fd.AntiAliased = false; // ClearType is non-deterministic, so disable it for screen-compare purposes
	HFONT hWebdings = oGDICreateFont(fd);

	oGUI_CONTROL_DESC b;
	b.hParent = (oGUI_WINDOW)_hParent;
	b.Type = oGUI_CONTROL_BUTTON;
	b.Size = kButtonSize;
	
	b.Text = "9";
	b.Position = _Position;
	b.ID = ID_BIG_BACK;
	HWND hWnd = oWinControlCreate(b);
	oWinControlSetFont(hWnd, hWebdings);

	b.Text = "7";
	b.Position += kButtonSpacing;
	b.ID = ID_BACK;
	hWnd = oWinControlCreate(b);
	oWinControlSetFont(hWnd, hWebdings);

	b.Text = "4";
	b.Position += kButtonSpacing;
	b.ID = ID_PLAY_PAUSE;
	hWnd = oWinControlCreate(b);
	oWinControlSetFont(hWnd, hWebdings);

	b.Text = "8";
	b.Position += kButtonSpacing;
	b.ID = ID_FORWARD;
	hWnd = oWinControlCreate(b);
	oWinControlSetFont(hWnd, hWebdings);

	b.Text = ":";
	b.Position += kButtonSpacing;
	b.ID = ID_BIG_FORWARD;
	hWnd = oWinControlCreate(b);
	oWinControlSetFont(hWnd, hWebdings);
}

oWindowUITest::oWindowUITest(bool* _pSuccess)
	: BorderStyle(oGUI_BORDER_SUNKEN)
	, Running(true)
{
	*_pSuccess = false;

	window::init i;
	i.title = "TESTWindowControls";
	i.event_hook = std::bind(&oWindowUITest::EventHook, this, std::placeholders::_1);
	i.action_hook = std::bind(&oWindowUITest::ActionHook, this, std::placeholders::_1);
	i.shape.State = oGUI_WINDOW_RESTORED;
	i.shape.Style = oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR;
	i.shape.ClientSize = int2(640,480);

	try { Window = window::make(i); }
	catch (std::exception& e) { oErrorSetLast(e); return; }

	int Widths[2] = { 100, oInvalid };
	Window->set_num_status_sections(Widths);

	// Disable anti-aliasing since on Windows ClearType seems to be non-deterministic
	Window->dispatch([&]
	{
		HWND hWnd = (HWND)Window->native_handle();
		oGUI_FONT_DESC fd;
		HFONT hCurrent = oWinGetFont(hWnd);
		oGDIGetFontDesc(hCurrent, &fd);
		fd.AntiAliased = false;
		HFONT hNew = oGDICreateFont(fd);
		oWinSetFont(hWnd, hNew);
	});

	Window->set_status_text(0, "OK");
	Window->set_status_text(1, "Solid");

	oGUI_HOTKEY_DESC_NO_CTOR HotKeys[] = 
	{
		{ oGUI_KEY_W, MENU_VIEW_WIREFRAME, false, true, false },
		{ oGUI_KEY_S, MENU_VIEW_SOLID, false, true, false },
	};

	Window->set_hotkeys(HotKeys);

	*_pSuccess = true;
}

bool oWindowUITest::OnCreate(HWND _hWnd, oGUI_MENU _hMenu)
{
	InitMenu(_hMenu);
	InitVCRControls(_hWnd, int2(150, 30));

	oGUI_CONTROL_DESC ButtonDesc;
	ButtonDesc.hParent = (oGUI_WINDOW)_hWnd;
	ButtonDesc.Type = oGUI_CONTROL_BUTTON;
	ButtonDesc.Text = "Push Me";
	ButtonDesc.Position = int2(10,10);
	ButtonDesc.Size = int2(100,20);
	ButtonDesc.ID = ID_PUSHME;
	Controls[ID_PUSHME] = oWinControlCreate(ButtonDesc);

	oGUI_CONTROL_DESC TextDesc;
	TextDesc.hParent = (oGUI_WINDOW)_hWnd;
	TextDesc.Type = oGUI_CONTROL_FLOATBOX;
	TextDesc.Text = "1.00";
	TextDesc.Position = int2(50,50);
	TextDesc.Size = int2(75,20);
	TextDesc.ID = ID_FLOATBOX;
	Controls[ID_FLOATBOX] = oWinControlCreate(TextDesc);

	oWinControlSetValue(Controls[ID_FLOATBOX], 1.234f);

	try
	{
		oWinControlSetText(Controls[ID_FLOATBOX], "Error!"); // should not show up
	}
	catch (std::exception&) {}

	TextDesc.Type = oGUI_CONTROL_FLOATBOX_SPINNER;
	TextDesc.Position.y += 40;
	TextDesc.ID = ID_FLOATBOX_SPINNER;
	Controls[ID_FLOATBOX_SPINNER] = oWinControlCreate(TextDesc);
	oVB(Controls[ID_FLOATBOX_SPINNER]);

	ButtonDesc.Text = "Easy";
	ButtonDesc.Position = int2(10,450);
	ButtonDesc.Size = int2(100,20);
	ButtonDesc.ID = ID_EASY;
	oWinControlCreate(ButtonDesc);

	oGUI_CONTROL_DESC SliderDesc;
	SliderDesc.hParent = (oGUI_WINDOW)_hWnd;
	SliderDesc.Type = oGUI_CONTROL_SLIDER;
	SliderDesc.Text = "Slider";
	SliderDesc.Position = int2(150,60);
	SliderDesc.Size = int2(140,25);
	SliderDesc.ID = ID_SLIDER;
	Controls[ID_SLIDER] = oWinControlCreate(SliderDesc);

	oVERIFY(oWinControlSetRange(Controls[ID_SLIDER], 20, 120));
	oVERIFY(oWinControlSetRangePosition(Controls[ID_SLIDER], 70));

	SliderDesc.Type = oGUI_CONTROL_SLIDER_SELECTABLE;
	SliderDesc.Text = "SliderSelectable";
	SliderDesc.Position = int2(150,90);
	SliderDesc.ID = ID_SLIDER_SELECTABLE;
	Controls[ID_SLIDER_SELECTABLE] = oWinControlCreate(SliderDesc);

	oVERIFY(oWinControlSetRange(Controls[ID_SLIDER_SELECTABLE], 0, 100));
	oVERIFY(oWinControlSelect(Controls[ID_SLIDER_SELECTABLE], 10, 50));
	oVERIFY(oWinControlSetRangePosition(Controls[ID_SLIDER_SELECTABLE], 30));

	oGUI_CONTROL_DESC ProgressBarDesc;
	ProgressBarDesc.hParent = (oGUI_WINDOW)_hWnd;
	ProgressBarDesc.Type = oGUI_CONTROL_PROGRESSBAR;
	ProgressBarDesc.Text = "ProgressBar";
	ProgressBarDesc.Position = int2(150,120);
	ProgressBarDesc.Size = int2(150,30);
	ProgressBarDesc.ID = ID_PROGRESSBAR;
	Controls[ID_PROGRESSBAR] = oWinControlCreate(ProgressBarDesc);
	
	oVERIFY(oWinControlSetRange(Controls[ID_PROGRESSBAR], 20, 30));
	oVERIFY(oWinControlSetRangePosition(Controls[ID_PROGRESSBAR], 25));
	oVERIFY(oWinControlSetErrorState(Controls[ID_PROGRESSBAR], true));

	// Marquee animates itself, making it a poor element of an automated image
	// test, so disable this for screen grabs, but show it's working in 
	// interactive mode
	if (kInteractiveMode)
	{
		ProgressBarDesc.Type = oGUI_CONTROL_PROGRESSBAR_UNKNOWN;
		ProgressBarDesc.Text = "Marquee";
		ProgressBarDesc.Position = int2(150,160);
		ProgressBarDesc.ID = ID_MARQUEE;
		Controls[ID_MARQUEE] = oWinControlCreate(ProgressBarDesc);
	}

	oGUI_CONTROL_DESC CB;
	CB.hParent = (oGUI_WINDOW)_hWnd;
	CB.Type = oGUI_CONTROL_COMBOBOX;
	CB.Text = "Init1|Init2|Init3";
	CB.Position = int2(50,120);
	CB.Size = int2(80,30);
	CB.ID = ID_COMBOBOX;
	Controls[ID_COMBOBOX] = oWinControlCreate(CB);

	oWinControlInsertSubItem(Controls[ID_COMBOBOX], "After3", oInvalid);
	oWinControlInsertSubItem(Controls[ID_COMBOBOX], "Btwn2And3", 2);

	CB.Type = oGUI_CONTROL_COMBOTEXTBOX;
	CB.Position = int2(50,160);
	CB.ID = ID_COMBOTEXTBOX;
	Controls[ID_COMBOTEXTBOX] = oWinControlCreate(CB);

	oWinControlInsertSubItem(Controls[ID_COMBOTEXTBOX], "After3", oInvalid);
	oWinControlInsertSubItem(Controls[ID_COMBOTEXTBOX], "Btwn2And3", 2);
	oWinControlSetText(Controls[ID_COMBOTEXTBOX], "Select...");

	return true;
}

void oWindowUITest::OnMenuCommand(HWND _hWnd, int _MenuID)
{
	bool StatusBarChanged = false;

	switch (_MenuID)
	{
		case MENU_FILE_OPEN:
		{
			ouro::path path;
			if (oWinDialogGetOpenPath(path, "TESTWindowUI", "Source Files|*.cpp|Header Files|*.h", _hWnd))
				oTRACE("Open %s", path.c_str());
			else
				oTRACE("Open dialog canceled.");

			ouro::msgbox(ouro::msg_type::info, nullptr, "TESTWindowUI", "User selected path:\n\t%s", path.c_str());
			break;
		}
		case MENU_FILE_SAVE:
			break;
		case MENU_FILE_SAVEAS:
		{
			ouro::path path;
			if (oWinDialogGetSavePath(path, "TESTWindowUI", "Source Files|*.cpp|Header Files|*.h", _hWnd))
				oTRACE("SaveAs %s", path.c_str());
			else
				oTRACE("SaveAs dialog canceled.");

			ouro::msgbox(ouro::msg_type::info, nullptr, "TESTWindowUI", "User selected path:\n\t%s", path.c_str());
			break;
		}
		case MENU_FILE_EXIT:
			Running = false;
			break;
		case MENU_EDIT_CUT:
			break;
		case MENU_EDIT_COPY:
			break;
		case MENU_EDIT_PASTE:
			break;
		case MENU_EDIT_COLOR:
		{
			ouro::color c = ouro::Red;

			if (!oWinDialogGetColor(&c, _hWnd))
				ouro::msgbox(ouro::msg_type::info, nullptr, "TESTWindowUI", "No color!");

			else
			{
				int r,g,b,a;
				c.decompose(&r, &g, &b, &a);
				ouro::msgbox(ouro::msg_type::info, nullptr, "TESTWindowUI", "Color: %d,%d,%d", r,g,b);
			}
			
			break;
		}
		case MENU_EDIT_FONT:
		{
			ouro::color c = ouro::Red;
			LOGFONT lf = {0};
			GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
			if (!oWinDialogGetFont(&lf, &c, _hWnd))
				ouro::msgbox(ouro::msg_type::info, nullptr, "TESTWindowUI", "No color!");

			else
			{
				int r,g,b,a;
				c.decompose(&r, &g, &b, &a);
				ouro::msgbox(ouro::msg_type::info, nullptr, "TESTWindowUI", "Color: %d,%d,%d", r,g,b);
			}
			
			break;
		}
		case MENU_VIEW_SOLID:
			oGUIMenuCheck(hViewMenu, MENU_VIEW_SOLID, true);
			oGUIMenuCheck(hViewMenu, MENU_VIEW_WIREFRAME, false);
			oGUIMenuEnable(hFileMenu, MENU_FILE_EXIT);
			Window->set_status_text(1, "Solid");
			break;
		case MENU_VIEW_WIREFRAME:
			oGUIMenuCheck(hViewMenu, MENU_VIEW_SOLID, false);
			oGUIMenuCheck(hViewMenu, MENU_VIEW_WIREFRAME, true);
			oGUIMenuEnable(hFileMenu, MENU_FILE_EXIT, false);
			Window->set_status_text(1, "Wireframe");
			break;
		case MENU_VIEW_SHOW_STATUSBAR:
		{
			bool NewState = !oGUIMenuIsChecked(hViewMenu, MENU_VIEW_SHOW_STATUSBAR);
			oGUIMenuCheck(hViewMenu, MENU_VIEW_SHOW_STATUSBAR, NewState);
			oGUI_WINDOW_SHAPE_DESC s;
			s.Style = NewState ? oGUI_WINDOW_SIZABLE_WITH_MENU_AND_STATUSBAR : oGUI_WINDOW_SIZABLE_WITH_MENU;
			Window->shape(s);
			break;
		}
		case MENU_HELP_ABOUT:
			break;
		default:
			break;
	}

	if (oGUIMenuIsChecked(hViewMenu, MENU_VIEW_SOLID))
		oTRACE("View Solid");
	else if (oGUIMenuIsChecked(hViewMenu, MENU_VIEW_WIREFRAME))
		oTRACE("View Wireframe");
	else
		oTRACE("View Nothing");

	oTRACE("Exit is %sabled", oGUIMenuIsEnabled(hFileMenu, MENU_FILE_EXIT) ? "en" : "dis");
}

void TESTWindowControls(test_services& _Services)
{
	if (ouro::system::is_remote_session())
		oTHROW(permission_denied, "Detected remote session: differing text anti-aliasing will cause bad image compares");

	bool success = false;
	oWindowUITest test(&success);
	if (!success)
		oTHROW(protocol_error, "failed to construct test window");

	double WaitForSettle = ouro::timer::now() + 1.0;
	do
	{
		test.GetWindow()->flush_messages();

	} while (ouro::timer::now() < WaitForSettle);

	oStd::future<std::shared_ptr<ouro::surface::buffer>> snapshot = test.GetWindow()->snapshot();
		
	do
	{
		test.GetWindow()->flush_messages();
		
	} while ((kInteractiveMode && test.GetRunning()) || !snapshot.is_ready());

	std::shared_ptr<ouro::surface::buffer> s = snapshot.get();
	_Services.check(s, 0);
}

	} // namespace tests
} // namespace ouro
