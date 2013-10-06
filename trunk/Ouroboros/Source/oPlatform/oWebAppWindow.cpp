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
#include <oPlatform/oWebAppWindow.h>
#include <oBasis/oLockThis.h>
#include <oPlatform/Windows/oWinWindowing.h>

enum OWEB_APP_WINDOW_CONTROLS
{
	ID_WEB_LINK,
	ID_NUM_JOBS,
	ID_CPU_UTIL,
	ID_STATIC_COUNT,
};

namespace ouro {

bool from_string(OWEB_APP_WINDOW_CONTROLS* _pValue, const char* _StrSource)
{
	if( 0 == _stricmp("ID_WEB_LINK", _StrSource) )
	{*_pValue = ID_WEB_LINK; return true;}
	else if( 0 == _stricmp("ID_NUM_JOBS", _StrSource) )
	{*_pValue = ID_NUM_JOBS; return true;}
	else if( 0 == _stricmp("ID_CPU_UTIL", _StrSource) )
	{*_pValue = ID_CPU_UTIL; return true;}
	return false;
}

} // namespace ouro

struct oCPUUsageStats
{
	oCPUUsageStats()
		: previousSystemTime(0)
		, previousProcessTime(0)
	{}

	unsigned long long previousSystemTime;
	unsigned long long previousProcessTime;
};

class oWebAppWindowImpl : public oWebAppWindow
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oWindow);

	oWebAppWindowImpl( const char* _pTitle, unsigned short _ServerPort, bool* _pSuccess );
	~oWebAppWindowImpl();

	void SetCurrentJobCount(uint _JobCount) threadsafe override;
	bool IsRunning() const threadsafe override { return Running; }
	void Close() threadsafe override { Running = false; }
	bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) threadsafe
	{
		unsigned int TimedOut = oTimerMS() + _TimeoutMS;
		bool IsTimedOut = false;
		while (Running && !IsTimedOut)
		{
			Window->FlushMessages();
			IsTimedOut = (_TimeoutMS != oInfiniteWait) && (oTimerMS() > TimedOut);
		}

		return !IsTimedOut;
	}

	// oWindow forwarding
	oGUI_WINDOW GetNativeHandle() const threadsafe override { return Window->GetNativeHandle(); }
	ouro::display::id GetDisplayId() const override { return Window->GetDisplayId(); }
	bool IsWindowThread() const threadsafe override { return Window->IsWindowThread(); }
	void SetShape(const oGUI_WINDOW_SHAPE_DESC& _Shape) threadsafe override { Window->SetShape(_Shape); }
	oGUI_WINDOW_SHAPE_DESC GetShape() const override { return Window->GetShape(); }
	void SetIcon(oGUI_ICON _hIcon) threadsafe override { Window->SetIcon(_hIcon); }
	oGUI_ICON GetIcon() const override { return Window->GetIcon(); }
	void SetUserCursor(oGUI_CURSOR _hCursor) threadsafe override { Window->SetUserCursor(_hCursor); }
	oGUI_CURSOR GetUserCursor() const override { return Window->GetUserCursor(); }
	void SetClientCursorState(oGUI_CURSOR_STATE _State) threadsafe override { Window->SetClientCursorState(_State); }
	oGUI_CURSOR_STATE GetClientCursorState() const override { return Window->GetClientCursorState(); }
	void SetTitleV(const char* _Format, va_list _Args) threadsafe override { return Window->SetTitleV(_Format, _Args); }
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const override { return Window->GetTitle(_StrDestination, _SizeofStrDestination); }
	void SetNumStatusSections(const int* _pStatusSectionWidths, size_t _NumStatusSections) threadsafe override { return Window->SetNumStatusSections(_pStatusSectionWidths, _NumStatusSections); }
	int GetNumStatusSections(int* _pStatusSectionWidths = nullptr, size_t _MaxNumStatusSectionWidths = 0) const override { return Window->GetNumStatusSections(_pStatusSectionWidths, _MaxNumStatusSectionWidths); }
	void SetStatusTextV(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe override { Window->SetStatusTextV(_StatusSectionIndex, _Format, _Args); }
	char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const override { return Window->GetStatusText(_StrDestination, _SizeofStrDestination, _StatusSectionIndex); }
	void SetStatusIcon(int _StatusSectionIndex, oGUI_ICON _hIcon) threadsafe override { Window->SetStatusIcon(_StatusSectionIndex, _hIcon); }
	oGUI_ICON GetStatusIcon(int _StatusSectionIndex) const override { return Window->GetStatusIcon(_StatusSectionIndex); }
	void SetParent(oWindow* _pParent) threadsafe override { Window->SetParent(_pParent); }
	oWindow* GetParent() const override { return Window->GetParent(); }
	void SetOwner(oWindow* _pOwner) threadsafe override { Window->SetOwner(_pOwner); }
	oWindow* GetOwner() const override { return Window->GetOwner(); }
	void SetSortOrder(oGUI_WINDOW_SORT_ORDER _SortOrder) threadsafe override { Window->SetSortOrder(_SortOrder); }
	oGUI_WINDOW_SORT_ORDER GetSortOrder() const override { return Window->GetSortOrder(); }
	void SetFocus(bool _Focus = true) threadsafe override { Window->SetFocus(_Focus); }
	bool HasFocus() const override { return Window->HasFocus(); }
	void SetDebug(bool _Debug = true) threadsafe override { Window->SetDebug(_Debug); }
	bool GetDebug() const override { return Window->GetDebug(); }
	void SetAllowTouchActions(bool _Allow = true) threadsafe override { Window->SetAllowTouchActions(_Allow); }
	bool GetAllowTouchActions() const override { return Window->GetAllowTouchActions(); }
	void SetClientDragToMove(bool _DragMoves = true) threadsafe override { Window->SetClientDragToMove(_DragMoves); }
	bool GetClientDragToMove() const override { return Window->GetClientDragToMove(); }
	void SetAltF4Closes(bool _AltF4Closes = true) threadsafe override { Window->SetAltF4Closes(_AltF4Closes); }
	bool GetAltF4Closes() const override { return Window->GetAltF4Closes(); }
	void SetEnabled(bool _Enabled) threadsafe override { Window->SetEnabled(_Enabled); }
	bool GetEnabled() const override { return Window->GetEnabled(); }
	void SetCapture(bool _Capture) threadsafe override { Window->SetCapture(_Capture); }
	bool HasCapture() const override { return Window->HasCapture(); }
	void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe override { Window->SetHotKeys(_pHotKeys, _NumHotKeys); }
	int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const override { return Window->GetHotKeys(_pHotKeys, _MaxNumHotKeys); }
	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override { return Window->HookActions(_Hook); }
	void UnhookActions(int _ActionHookID) threadsafe override { return Window->UnhookActions(_ActionHookID); }
	int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe override { return Window->HookEvents(_Hook); }
	void UnhookEvents(int _EventHookID) threadsafe override { return Window->UnhookEvents(_EventHookID); }
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override { return Window->Trigger(_Action); }
	void Post(int _CustomEventCode, uintptr_t _Context) threadsafe override { return Window->Post(_CustomEventCode, _Context); }
	void Dispatch(const oTASK& _Task) threadsafe override { return Window->Dispatch(_Task); }
	oStd::future<std::shared_ptr<ouro::surface::buffer>> CreateSnapshot(int _Frame = oInvalid, bool _IncludeBorder = false) threadsafe const override { return std::move(Window->CreateSnapshot(_Frame, _IncludeBorder)); }
	void SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe override { Window->SetTimer(_Context, _RelativeTimeMS); }
	void StopTimer(uintptr_t _Context) threadsafe override { return Window->StopTimer(_Context); }
	void FlushMessages(bool _WaitForNext = false) override { return Window->FlushMessages(_WaitForNext); }
	void Quit() threadsafe override { Window->Quit(); }

	void OnEvent(const oGUI_EVENT_DESC& _Event);
private:
	oRefCount RefCount;
	ouro::intrusive_ptr<oWindow> Window;
	HWND LinkHandle;
	HWND CPUUtilization;
	HWND NumberOfJobs;
	bool Running;

	oInitOnce<unsigned short> ServerPort;
	oInitOnce<ouro::sstring> Title;

	oCPUUsageStats CPUUsageStats;
};

bool oWebAppWindowCreate( const char* _pTitle, unsigned short _ServerPort, oWebAppWindow** _ppWebAppWindow )
{
	bool success = false;
	oCONSTRUCT(_ppWebAppWindow, oWebAppWindowImpl(_pTitle, _ServerPort, &success));
	return success;
}

oWebAppWindowImpl::oWebAppWindowImpl( const char* _pTitle, unsigned short _ServerPort, bool* _pSuccess )
	: Running(true)
{
	Title.Initialize(_pTitle);
	ServerPort.Initialize(_ServerPort);

	oWINDOW_INIT init;
	init.Title = _pTitle;
	init.EventHook = oBIND(&oWebAppWindowImpl::OnEvent, this, oBIND1);
	init.ActionHook = nullptr;
	init.Shape.Style = oGUI_WINDOW_FIXED;
	init.Shape.ClientSize = int2(250,90);
	//init.Shape.EnableMainLoopEvent = true;

	// Create the base window
	if (!oWindowCreate(init, &Window))
	{
		oErrorPrefixLast("Failed to create base window with ");
		return;
	}

	Window->SetTimer((uintptr_t)&CPUUsageStats, 500);

	*_pSuccess = true;
}

oWebAppWindowImpl::~oWebAppWindowImpl()
{
	Window = nullptr;
}

void oWebAppWindowImpl::SetCurrentJobCount(uint _JobCount) threadsafe
{
	ouro::sstring jobCountString;
	snprintf(jobCountString, "Number of Jobs: %d", _JobCount);
	Window->Dispatch([=](){
		oWinControlSetText(NumberOfJobs, jobCountString);
	});
};

void oWebAppWindowImpl::OnEvent(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_CREATING:
		{
			oGUI_CONTROL_DESC ControlDesc;
			ControlDesc.hParent = (oGUI_WINDOW)_Event.hWindow;

			// Make the link
			{
				ouro::mstring localHostString;
				snprintf(localHostString, "<a href=\"http://localhost:%d/\">Launch %s App</a>", *ServerPort, Title);

				ControlDesc.Position = int2(45, 50);
				ControlDesc.Size = int2(200, 30);
				ControlDesc.ID = ID_WEB_LINK;
				ControlDesc.Type = oGUI_CONTROL_HYPERLABEL;
				ControlDesc.Text = localHostString.c_str();
				LinkHandle = oWinControlCreate(ControlDesc);
			}
			
			ControlDesc.Type = oGUI_CONTROL_LABEL;

			// Add CPU time
			{
				ControlDesc.Position = int2(130, 20);
				ControlDesc.Size = int2(150, 20);
				ControlDesc.ID = ID_WEB_LINK;
				
				ControlDesc.Text = "CPU Utilization";
				CPUUtilization = oWinControlCreate(ControlDesc);
			}

			// Number of Jobs
			{
				ControlDesc.Position = int2(30, 20);
				ControlDesc.Size = int2(100, 20);
				ControlDesc.ID = ID_WEB_LINK;

				ControlDesc.Text = "Number of Jobs";
				NumberOfJobs = oWinControlCreate(ControlDesc);
			}
			break;
		}

		case oGUI_TIMER:
		{
			if (_Event.AsTimer().Context == (uintptr_t)&CPUUsageStats)
			{
				double usage = ouro::this_process::cpu_usage(&CPUUsageStats.previousSystemTime, &CPUUsageStats.previousProcessTime);
				ouro::sstring usageString;
				snprintf(usageString, "CPU Utilization: %.0f%%", usage);
				oWinControlSetText(CPUUtilization, usageString);
			}

			break;
		}

		case oGUI_CLOSING:
			Running = false;
			break;
	}
}