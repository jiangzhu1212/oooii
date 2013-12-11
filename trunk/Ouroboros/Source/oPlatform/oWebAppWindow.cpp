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
#include <oGUI/Windows/oWinWindowing.h>

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
	oDEFINE_NOOP_QUERYINTERFACE();

	oWebAppWindowImpl( const char* _pTitle, unsigned short _ServerPort, bool* _pSuccess );
	~oWebAppWindowImpl();

	void SetCurrentJobCount(uint _JobCount) override;
	bool IsRunning() const override { return Running; }
	void Close() override { Running = false; }
	bool WaitUntilClosed(unsigned int _TimeoutMS = oInfiniteWait)
	{
		unsigned int TimedOut = ouro::timer::now_ms() + _TimeoutMS;
		bool IsTimedOut = false;
		while (Running && !IsTimedOut)
		{
			thread_cast<oWebAppWindowImpl*>(this)->Window->flush_messages();
			IsTimedOut = (_TimeoutMS != oInfiniteWait) && (ouro::timer::now_ms() > TimedOut);
		}

		return !IsTimedOut;
	}

	// oWindow forwarding
	oGUI_WINDOW native_handle() const override { return Window->native_handle(); }
	ouro::display::id display_id() const override { return Window->display_id(); }
	bool is_window_thread() const override { return Window->is_window_thread(); }
	void shape(const oGUI_WINDOW_SHAPE_DESC& _Shape) override { Window->shape(_Shape); }
	oGUI_WINDOW_SHAPE_DESC shape() const override { return Window->shape(); }
	void icon(oGUI_ICON _hIcon) override { Window->icon(_hIcon); }
	oGUI_ICON icon() const override { return Window->icon(); }
	void user_cursor(oGUI_CURSOR _hCursor) override { Window->user_cursor(_hCursor); }
	oGUI_CURSOR user_cursor() const override { return Window->user_cursor(); }
	void client_cursor_state(oGUI_CURSOR_STATE _State) override { Window->client_cursor_state(_State); }
	oGUI_CURSOR_STATE client_cursor_state() const override { return Window->client_cursor_state(); }
	void set_titlev(const char* _Format, va_list _Args) override { return Window->set_titlev(_Format, _Args); }
	char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const override { return Window->get_title(_StrDestination, _SizeofStrDestination); }
	void set_num_status_sections(const int* _pStatusSectionWidths, size_t _NumStatusSections) override { return Window->set_num_status_sections(_pStatusSectionWidths, _NumStatusSections); }
	int get_num_status_sections(int* _pStatusSectionWidths = nullptr, size_t _MaxNumStatusSectionWidths = 0) const override { return Window->get_num_status_sections(_pStatusSectionWidths, _MaxNumStatusSectionWidths); }
	void set_status_textv(int _StatusSectionIndex, const char* _Format, va_list _Args) override { Window->set_status_textv(_StatusSectionIndex, _Format, _Args); }
	char* get_status_text(char* _StrDestination, size_t _SizeofStrDestination, int _StatusSectionIndex) const override { return Window->get_status_text(_StrDestination, _SizeofStrDestination, _StatusSectionIndex); }
	void status_icon(int _StatusSectionIndex, oGUI_ICON _hIcon) override { Window->status_icon(_StatusSectionIndex, _hIcon); }
	oGUI_ICON status_icon(int _StatusSectionIndex) const override { return Window->status_icon(_StatusSectionIndex); }
	void parent(const std::shared_ptr<basic_window>& _Parent) override { Window->parent(_Parent); }
	std::shared_ptr<basic_window> parent() const override { return Window->parent(); }
	void owner(const std::shared_ptr<basic_window>& _Owner) override { Window->owner(_Owner); }
	std::shared_ptr<basic_window> owner() const override { return Window->owner(); }
	void sort_order(oGUI_WINDOW_SORT_ORDER _SortOrder) override { Window->sort_order(_SortOrder); }
	oGUI_WINDOW_SORT_ORDER sort_order() const override { return Window->sort_order(); }
	void focus(bool _Focus = true) override { Window->focus(_Focus); }
	bool has_focus() const override { return Window->has_focus(); }
	void debug(bool _Debug = true) override { Window->debug(_Debug); }
	bool debug() const override { return Window->debug(); }
	void allow_touch_actions(bool _Allow) override { Window->allow_touch_actions(_Allow); }
	bool allow_touch_actions() const override { return Window->allow_touch_actions(); }
	void client_drag_to_move(bool _DragMoves) override { Window->client_drag_to_move(_DragMoves); }
	bool client_drag_to_move() const override { return Window->client_drag_to_move(); }
	void alt_f4_closes(bool _AltF4Closes) override { Window->alt_f4_closes(_AltF4Closes); }
	bool alt_f4_closes() const override { return Window->alt_f4_closes(); }
	void enabled(bool _Enabled) override { Window->enabled(_Enabled); }
	bool enabled() const override { return Window->enabled(); }
	void capture(bool _Capture) override { Window->capture(_Capture); }
	bool has_capture() const override { return Window->has_capture(); }
	void set_hotkeys(const oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _NumHotKeys) override { Window->set_hotkeys(_pHotKeys, _NumHotKeys); }
	int get_hotkeys(oGUI_HOTKEY_DESC_NO_CTOR* _pHotKeys, size_t _MaxNumHotKeys) const override { return Window->get_hotkeys(_pHotKeys, _MaxNumHotKeys); }
	int hook_actions(const oGUI_ACTION_HOOK& _Hook) override { return Window->hook_actions(_Hook); }
	void unhook_actions(int _ActionHookID) override { return Window->unhook_actions(_ActionHookID); }
	int hook_events(const oGUI_EVENT_HOOK& _Hook) override { return Window->hook_events(_Hook); }
	void unhook_events(int _EventHookID) override { return Window->unhook_events(_EventHookID); }
	void trigger(const oGUI_ACTION_DESC& _Action) override { return Window->trigger(_Action); }
	void post(int _CustomEventCode, uintptr_t _Context) override { return Window->post(_CustomEventCode, _Context); }
	void dispatch(const oTASK& _Task) override { return Window->dispatch(_Task); }
	oStd::future<std::shared_ptr<ouro::surface::buffer>> snapshot(int _Frame = oInvalid, bool _IncludeBorder = false) const override { return Window->snapshot(_Frame, _IncludeBorder); }
	void start_timer(uintptr_t _Context, unsigned int _RelativeTimeMS) override { Window->start_timer(_Context, _RelativeTimeMS); }
	void stop_timer(uintptr_t _Context) override { return Window->stop_timer(_Context); }
	void flush_messages(bool _WaitForNext = false) override { return Window->flush_messages(_WaitForNext); }
	void quit() override { Window->quit(); }

	void OnEvent(const oGUI_EVENT_DESC& _Event);
private:
	oRefCount RefCount;
	std::shared_ptr<window> Window;
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

	window::init init;
	init.title = _pTitle;
	init.event_hook = oBIND(&oWebAppWindowImpl::OnEvent, this, oBIND1);
	init.action_hook = nullptr;
	init.shape.Style = oGUI_WINDOW_FIXED;
	init.shape.ClientSize = int2(250,90);
	//init.shape.EnableMainLoopEvent = true;

	try { Window = window::make(init); }
	catch (std::exception& e)
	{
		oErrorSetLast(e);
		oErrorPrefixLast("Failed to create base window with ");
		return;
	}
	Window->start_timer((uintptr_t)&CPUUsageStats, 500);

	*_pSuccess = true;
}

oWebAppWindowImpl::~oWebAppWindowImpl()
{
	Window = nullptr;
}

void oWebAppWindowImpl::SetCurrentJobCount(uint _JobCount)
{
	ouro::sstring jobCountString;
	snprintf(jobCountString, "Number of Jobs: %d", _JobCount);
	Window->dispatch([=](){
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