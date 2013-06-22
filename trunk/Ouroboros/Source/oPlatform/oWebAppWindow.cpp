/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/oProcess.h>
#include <oPlatform/Windows/oWinWindowing.h>

enum OWEB_APP_WINDOW_CONTROLS
{
	ID_WEB_LINK,
	ID_NUM_JOBS,
	ID_CPU_UTIL,
	ID_STATIC_COUNT,
};

namespace oStd {

bool from_string(OWEB_APP_WINDOW_CONTROLS* _pValue, const char* _StrSource)
{
	if( 0 == oStricmp("ID_WEB_LINK", _StrSource) )
	{*_pValue = ID_WEB_LINK; return true;}
	else if( 0 == oStricmp("ID_NUM_JOBS", _StrSource) )
	{*_pValue = ID_NUM_JOBS; return true;}
	else if( 0 == oStricmp("ID_CPU_UTIL", _StrSource) )
	{*_pValue = ID_CPU_UTIL; return true;}
	return false;
}

} // namespace oStd

class oWebAppWindowImpl : public oWebAppWindow
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oWindow);

	oWebAppWindowImpl( const char* _pTitle, unsigned short _ServerPort, bool* _pSuccess );
	~oWebAppWindowImpl();

	void SetCurrentJobCount(uint _JobCount) threadsafe override;

	// oWindow forwarding
	oGUI_WINDOW GetNativeHandle() const threadsafe { return Window->GetNativeHandle(); }
	bool IsOpen() const threadsafe override { return Window->IsOpen(); }
	bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override { return Window->WaitUntilClosed(_TimeoutMS); }
	bool WaitUntilOpaque(unsigned int _TimeoutMS = oInfiniteWait) const threadsafe override { return Window->WaitUntilOpaque(_TimeoutMS); }
	bool Close() threadsafe override { return Window->Close(); }
	bool IsWindowThread() const threadsafe override { return Window->IsWindowThread(); }
	bool HasFocus() const threadsafe override { return Window->HasFocus(); }
	void SetFocus() threadsafe override { return Window->SetFocus(); }
	void GetDesc(oGUI_WINDOW_DESC* _pDesc) const threadsafe override { Window->GetDesc(_pDesc); }
	bool Map(oGUI_WINDOW_DESC** _ppDesc) threadsafe override { return Window->Map(_ppDesc); }
	void Unmap() threadsafe override { Window->Unmap(); }
	void SetDesc(const oGUI_WINDOW_CURSOR_DESC& _pDesc) threadsafe override { Window->SetDesc(_pDesc); }
	void GetDesc(oGUI_WINDOW_CURSOR_DESC* _pDesc) const threadsafe override { Window->GetDesc(_pDesc); }
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe override { return Window->GetTitle(_StrDestination, _SizeofStrDestination); }
	void SetTitleV(const char* _Format, va_list _Args) threadsafe override { Window->SetTitleV(_Format, _Args); }
	char* GetStatusText(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const threadsafe override { return Window->GetStatusText(_StrDestination, _SizeofStrDestination, _StatusSectionIndex); }
	void SetStatusText(int _StatusSectionIndex, const char* _Format, va_list _Args) threadsafe override { Window->SetStatusText(_StatusSectionIndex, _Format, _Args); }
	void SetHotKeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) threadsafe override { Window->SetHotKeys(_pHotKeys, _NumHotKeys); }
	int GetHotKeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const threadsafe override { return Window->GetHotKeys(_pHotKeys, _MaxNumHotKeys); }
	void Trigger(const oGUI_ACTION_DESC& _Action) threadsafe override { Window->Trigger(_Action); }
	void Dispatch(const oTASK& _Task) threadsafe override { Window->Dispatch(_Task); }
	void SetTimer(uintptr_t _Context, unsigned int _RelativeTimeMS) threadsafe override { Window->SetTimer(_Context, _RelativeTimeMS); }
	int HookActions(const oGUI_ACTION_HOOK& _Hook) threadsafe override { return Window->HookActions(_Hook); }
	void UnhookActions(int _ActionHookID) threadsafe override { Window->UnhookActions(_ActionHookID); }
	int HookEvents(const oGUI_EVENT_HOOK& _Hook) threadsafe override { return Window->HookEvents(_Hook); }
	void UnhookEvents(int _EventHookID) threadsafe override { Window->UnhookEvents(_EventHookID); }
	oStd::future<oRef<oImage>> CreateSnapshot(int _Frame = oInvalid, bool _IncludeBorder = false) const threadsafe override{return Window->CreateSnapshot(_Frame, _IncludeBorder);}

	bool EventHook(const oGUI_EVENT_DESC& _Event);
private:
	oRefCount RefCount;
	oRef<threadsafe oWindow> Window;
	HWND LinkHandle;
	HWND CPUUtilization;
	HWND NumberOfJobs;

	oInitOnce<unsigned short> ServerPort;
	oInitOnce<oStd::sstring> Title;
	unsigned long long previousSystemTime;
	unsigned long long previousProcessTime;
};

bool oWebAppWindowCreate( const char* _pTitle, unsigned short _ServerPort, oWebAppWindow** _ppWebAppWindow )
{
	bool success = false;
	oCONSTRUCT(_ppWebAppWindow, oWebAppWindowImpl(_pTitle, _ServerPort, &success));
	return success;
}

oWebAppWindowImpl::oWebAppWindowImpl( const char* _pTitle, unsigned short _ServerPort, bool* _pSuccess )
	: previousSystemTime(0)
	, previousProcessTime(0)
{
	Title.Initialize(_pTitle);
	ServerPort.Initialize(_ServerPort);

	oWINDOW_INIT init;
	init.WindowTitle = _pTitle;
	init.EventHook = oBIND(&oWebAppWindowImpl::EventHook, this, oBIND1);
	init.ActionHook = nullptr;
	init.WinDesc.Style = oGUI_WINDOW_FIXED;
	init.WinDesc.ClientSize = int2(250,90);
	init.WinDesc.EnableMainLoopEvent = true;

	// Create the base window
	if (!oWindowCreate(init, &Window))
	{
		oErrorPrefixLast("Failed to create base window with ");
		return;
	}

	*_pSuccess = true;
}

oWebAppWindowImpl::~oWebAppWindowImpl()
{
	Window = nullptr;
}

void oWebAppWindowImpl::SetCurrentJobCount(uint _JobCount) threadsafe
{
	oStd::sstring jobCountString;
	oPrintf(jobCountString, "Number of Jobs: %d", _JobCount);
	Window->Dispatch([=](){
		oWinControlSetText(NumberOfJobs, jobCountString);
	});
};

bool oWebAppWindowImpl::EventHook(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Event)
	{
	case oGUI_CREATING:
		{
			oGUI_CONTROL_DESC ControlDesc;
			ControlDesc.hParent = (oGUI_WINDOW)_Event.hWindow;

			// Make the link
			{
				oStd::mstring localHostString;
				oPrintf(localHostString, "<a href=\"http://localhost:%d/\">Launch %s App</a>", *ServerPort, Title);

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
		}

	case oGUI_MAINLOOP:
		{
			//Update cpu utilization
			double usage = oProcessCalculateCPUUsage(oProcessGetCurrentID(), &previousSystemTime, &previousProcessTime);
			oStd::sstring usageString;
			oPrintf(usageString, "CPU Utilization: %.0f%%", usage);
			oWinControlSetText(CPUUtilization, usageString);
			oSleep(500);
		}
	}

	return true;
}