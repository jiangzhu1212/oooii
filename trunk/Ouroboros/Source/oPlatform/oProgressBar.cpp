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
#include <oPlatform/oProgressBar.h>
#include <oPlatform/Windows/oWinRect.h>
#include <oPlatform/Windows/oWinWindowing.h>

struct oWinProgressBar : oProgressBar
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oWinProgressBar(const oTASK& _OnStopPressed, const char* _Title = "", oGUI_ICON _hIcon = nullptr);
	void ShowStop(bool _Show = true) threadsafe override { Window->Dispatch([=] { oWinControlSetVisible(Get(PB_BUTTON), _Show); }); }
	bool StopShown() const override { return oWinControlIsVisible(Get(PB_BUTTON)); }
	void SetStopped(bool _Stopped = true) threadsafe override;
	bool GetStopped() const override { return oWinControlGetErrorState(Get(PB_PROGRESS)); }
	void SetTextV(const char* _Format, va_list _Args) threadsafe override;
	char* GetText(char* _StrDestination, size_t _SizeofStrDestination) const override { return oWinControlGetText(_StrDestination, _SizeofStrDestination, Get(PB_TEXT)); }
	void SetSubtextV(const char* _Format, va_list _Args) threadsafe override;
	char* GetSubtext(char* _StrDestination, size_t _SizeofStrDestination) const override { return oWinControlGetText(_StrDestination, _SizeofStrDestination, Get(PB_SUBTEXT)); }
	void SetPercentage(int _Percentage) threadsafe override { Window->Dispatch([=] { oThreadsafe(this)->SetPercentageInternal(_Percentage); }); }
	void AddPercentage(int _Percentage) threadsafe override { Window->Dispatch([=] { oThreadsafe(this)->SetPercentageInternal(oThreadsafe(this)->GetPercentage() + _Percentage); }); }
	int GetPercentage() const override { return oWinControlGetRangePosition(Get(PB_PROGRESS)); }
	const threadsafe oWindow* GetWindow() const threadsafe override { return Window; }
	threadsafe oWindow* GetWindow() threadsafe override { return Window; }
	const oWindow* GetWindow() const override { return Window; }
	oWindow* GetWindow() override { return Window; }

private:
	oRef<oWindow> Window;
	oTASK OnStopPressed;

	enum PB_CONTROL
	{
		PB_TEXT,
		PB_SUBTEXT,
		PB_BUTTON,
		PB_PERCENT,
		PB_MARQUEE,
		PB_PROGRESS,
		PB_CONTROL_COUNT,
	};

	std::array<oGUI_WINDOW, PB_CONTROL_COUNT> Controls;
	oRefCount RefCount;
	int Percentage;

private:
	void OnEvent(const oGUI_EVENT_DESC& _Event);
	void OnAction(const oGUI_ACTION_DESC& _Action);
	bool CreateControls(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
	HWND Get(PB_CONTROL _Control) const threadsafe { return (HWND)oThreadsafe(this)->Controls[_Control]; }
	void SetPercentageInternal(HWND _hProgress, HWND _hMarquee, HWND _hPercent, int _Percentage);
	void SetPercentageInternal(int _Percentage) { SetPercentageInternal(Get(PB_PROGRESS), Get(PB_MARQUEE), Get(PB_PERCENT), _Percentage); }
};

oWinProgressBar::oWinProgressBar(const oTASK& _OnStopPressed, const char* _Title, oGUI_ICON _hIcon)
	: OnStopPressed(_OnStopPressed)
	, Percentage(-1)
{
	Controls.fill(nullptr);

	oWINDOW_INIT Init;
	Init.Title = _Title;
	Init.hIcon = _hIcon;
	Init.ActionHook = oBIND(&oWinProgressBar::OnAction, this, oBIND1);
	Init.EventHook = oBIND(&oWinProgressBar::OnEvent, this, oBIND1);
	Init.Shape.ClientSize = int2(320, 106);
	Init.Shape.State = oGUI_WINDOW_HIDDEN;
	Init.Shape.Style = oGUI_WINDOW_FIXED;
	if (!oWindowCreate(Init, &Window))
		oThrowLastError();

	SetPercentageInternal(-1);
}

bool oProgressBarCreate(const oTASK& _OnStopPressed, const char* _Title, oGUI_ICON _hIcon, oProgressBar** _ppProgressBar)
{
	try { *_ppProgressBar = new oWinProgressBar(_OnStopPressed, _Title, _hIcon); }
	catch (std::exception& e) { *_ppProgressBar = nullptr; return oErrorSetLast(e); }
	return true;
}

bool oWinProgressBar::CreateControls(const oGUI_EVENT_CREATE_DESC& _CreateEvent)
{
	const int2 ProgressBarSize(270, 22);
	const int2 ButtonSize(75, 23);

	const int2 Inset(10, 10);
	const oRECT rParent = oRect(oWinRectWH(int2(0,0), _CreateEvent.Shape.ClientSize));

	oGUI_CONTROL_DESC Descs[PB_CONTROL_COUNT];

	// progress/marquee bars
	{
		oRECT rChild = oRect(oWinRectWH(int2(Inset.x, 0), ProgressBarSize));
		oRECT rText = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_MIDDLE_LEFT, true);
		Descs[PB_MARQUEE].Type = oGUI_CONTROL_PROGRESSBAR_UNKNOWN;
		Descs[PB_MARQUEE].Position = oWinRectPosition(oWinRect(rText));
		Descs[PB_MARQUEE].Size = ProgressBarSize;

		Descs[PB_PROGRESS].Type = oGUI_CONTROL_PROGRESSBAR;
		Descs[PB_PROGRESS].Position = oWinRectPosition(oWinRect(rText));
		Descs[PB_PROGRESS].Size = ProgressBarSize;
	}

	// percentage text
	{
		const auto& cpb = Descs[PB_PROGRESS];
		Descs[PB_PERCENT].Type = oGUI_CONTROL_LABEL;
		Descs[PB_PERCENT].Text = "0%";
		Descs[PB_PERCENT].Position = int2(cpb.Position.x + cpb.Size.x + 10, cpb.Position.y + 3);
		Descs[PB_PERCENT].Size = int2(35, cpb.Size.y);
	}

	// Stop button
	{
		oRECT rChild = oRect(oWinRectWH(-Inset, ButtonSize));
		oRECT rButton = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_BOTTOM_RIGHT, true);

		Descs[PB_BUTTON].Type = oGUI_CONTROL_BUTTON;
		Descs[PB_BUTTON].Text = "&Stop";
		Descs[PB_BUTTON].Position = oWinRectPosition(oWinRect(rButton));
		Descs[PB_BUTTON].Size = oWinRectSize(oWinRect(rButton));
	}

	// text/subtext
	{
		Descs[PB_TEXT].Type = oGUI_CONTROL_LABEL_CENTERED;
		Descs[PB_TEXT].Position = Inset;
		Descs[PB_TEXT].Size = int2(_CreateEvent.Shape.ClientSize.x - 2*Inset.x, 20);

		Descs[PB_SUBTEXT].Type = oGUI_CONTROL_LABEL;
		Descs[PB_SUBTEXT].Position = int2(Inset.x, Descs[PB_PROGRESS].Position.y + Descs[PB_PROGRESS].Size.y + 5);
		Descs[PB_SUBTEXT].Size = int2((Descs[PB_PROGRESS].Size.x * 3) / 4, 20);
	}

	for (short i = 0; i < PB_CONTROL_COUNT; i++)
	{
		Descs[i].hParent = _CreateEvent.hWindow;
		Descs[i].ID = i;
		Controls[i] = (oGUI_WINDOW)oWinControlCreate(Descs[i]);
	}

	return true;
}

void oWinProgressBar::OnEvent(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_CREATING:
		{
			if (!CreateControls(_Event.AsCreate()))
				oThrowLastError();
			break;
		}

		default:
			break;
	}
}

void oWinProgressBar::OnAction(const oGUI_ACTION_DESC& _Action)
{
	if (_Action.Action == oGUI_ACTION_CONTROL_ACTIVATED && _Action.DeviceID == PB_BUTTON && OnStopPressed)
	{
		SetStopped(true);
		OnStopPressed();
	}
}

void oWinProgressBar::SetStopped(bool _Stopped) threadsafe
{
	Window->Dispatch([=]
	{
		oWinControlSetErrorState(Get(PB_PROGRESS), _Stopped);
		oWinControlSetErrorState(Get(PB_MARQUEE), _Stopped);
	});
}

void oWinProgressBar::SetTextV(const char* _Format, va_list _Args) threadsafe
{
	oStd::lstring s;
	if (-1 == vsnprintf(s, _Format, _Args))
	{
		oStd::ellipsize(s);
		oTHROW0(no_buffer_space);
	}
	Window->Dispatch([=] { oWinControlSetText(Get(PB_TEXT), s.c_str()); });
}

void oWinProgressBar::SetSubtextV(const char* _Format, va_list _Args) threadsafe
{
	oStd::lstring s;
	if (-1 == vsnprintf(s, _Format, _Args))
	{
		oStd::ellipsize(s);
		oTHROW0(no_buffer_space);
	}
	Window->Dispatch([=] { oWinControlSetText(Get(PB_SUBTEXT), s.c_str()); });
}

static void EnsureVisible(HWND _hControl)
{
	if (!oWinControlIsVisible(_hControl))
		oWinControlSetVisible(_hControl);
}

static void EnsureHidden(HWND _hControl)
{
	if (oWinControlIsVisible(_hControl))
		oWinControlSetVisible(_hControl, false);
}

void oWinProgressBar::SetPercentageInternal(HWND _hProgress, HWND _hMarquee, HWND _hPercent, int _Percentage)
{
	if (_Percentage < 0)
	{
		EnsureHidden(_hProgress);
		EnsureHidden(_hPercent);
		EnsureVisible(_hMarquee);
	}

	else
	{
		UINT p = (UINT)__max(0, __min(100, (UINT)_Percentage));
		oWinControlSetRangePosition(_hProgress, p);

		char buf[16];
		snprintf(buf, "%u%%", p);
		oStd::ellipsize(buf);
		oVERIFY(oWinControlSetText(_hPercent, buf));

		EnsureHidden(_hMarquee);
		EnsureVisible(_hProgress);
		EnsureVisible(_hPercent);
	}

	Percentage = _Percentage;
}
