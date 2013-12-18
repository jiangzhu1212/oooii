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
#include <oGUI/progress_bar.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>

namespace ouro {

class progress_bar_impl : public progress_bar
{
public:
	progress_bar_impl(const char* _Title, ouro::icon_handle _hIcon
		, const std::function<void()>& _OnStop);


	// basic_window API
	ouro::window_handle native_handle() const override { return Window->native_handle(); }
	display::id display_id() const override { return Window->display_id(); }
	bool is_window_thread() const override { return Window->is_window_thread(); }
	void flush_messages(bool _WaitForNext = false) override { Window->flush_messages(_WaitForNext); }
	void quit() override { Window->quit(); }
	void debug(bool _Debug) override { Window->debug(_Debug); }
	bool debug() const override { return Window->debug(); }
	void state(window_state::value _State) override { Window->state(_State); }
	window_state::value state() const override { return Window->state(); }
	void client_position(const int2& _ClientPosition) override { Window->client_position(_ClientPosition); }
	int2 client_position() const override { return Window->client_position(); }
	int2 client_size() const override { return Window->client_size(); }
	void icon(ouro::icon_handle _hIcon) override { Window->icon(_hIcon); }
	ouro::icon_handle icon() const override { return Window->icon(); }
	void user_cursor(ouro::cursor_handle _hCursor) override { Window->user_cursor(_hCursor); }
	ouro::cursor_handle user_cursor() const override { return Window->user_cursor(); }
	void client_cursor_state(cursor_state::value _State) override { Window->client_cursor_state(_State); }
	cursor_state::value client_cursor_state() const override { return Window->client_cursor_state(); }
	void set_titlev(const char* _Format, va_list _Args) override { Window->set_titlev(_Format, _Args); }
	char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const override { return Window->get_title(_StrDestination, _SizeofStrDestination); }
	void parent(const std::shared_ptr<basic_window>& _Parent) override { Window->parent(_Parent); }
	std::shared_ptr<basic_window> parent() const override { return Window->parent(); }
	void owner(const std::shared_ptr<basic_window>& _Owner) override { Window->owner(_Owner); }
	std::shared_ptr<basic_window> owner() const override { return Window->owner(); }
	void sort_order(window_sort_order::value _SortOrder) override { Window->sort_order(_SortOrder); }
	window_sort_order::value sort_order() const override { return Window->sort_order(); }
	void focus(bool _Focus) override { Window->focus(_Focus); }
	bool has_focus() const override { return Window->has_focus(); }

	// progress_bar API
	void stop_button(bool _Show) override { Window->dispatch([=] { oWinControlSetVisible(get(PB_BUTTON), _Show); }); }
	bool stop_button() const override { return oWinControlIsVisible(get(PB_BUTTON)); }
	void stopped(bool _Stopped = true) override;
	bool stopped() const override { return oWinControlGetErrorState(get(PB_PROGRESS)); }
	void set_textv(const char* _Format, va_list _Args) override;
	char* get_text(char* _StrDestination, size_t _SizeofStrDestination) const override { return oWinControlGetText(_StrDestination, _SizeofStrDestination, get(PB_TEXT)); }
	void set_subtextv(const char* _Format, va_list _Args) override;
	char* get_subtext(char* _StrDestination, size_t _SizeofStrDestination) const override { return oWinControlGetText(_StrDestination, _SizeofStrDestination, get(PB_SUBTEXT)); }
	void set_percentage(int _Percentage) override { Window->dispatch([=] { set_percentage_internal(_Percentage); }); }
	void add_percentage(int _Percentage) override { Window->dispatch([=] { set_percentage_internal(percentage() + _Percentage); }); }
	int percentage() const override { return oWinControlGetRangePosition(get(PB_PROGRESS)); }
	
private:
	std::shared_ptr<window> Window;
	std::function<void()> OnStop;

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

	std::array<ouro::window_handle, PB_CONTROL_COUNT> Controls;
	int Percentage;

private:
	void on_event(const oGUI_EVENT_DESC& _Event);
	void on_action(const oGUI_ACTION_DESC& _Action);
	void make_controls(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
	HWND get(PB_CONTROL _Control) const { return (HWND)oThreadsafe(this)->Controls[_Control]; }
	void set_percentage_internal(HWND _hProgress, HWND _hMarquee, HWND _hPercent, int _Percentage);
	void set_percentage_internal(int _Percentage) { set_percentage_internal(get(PB_PROGRESS), get(PB_MARQUEE), get(PB_PERCENT), _Percentage); }
};

progress_bar_impl::progress_bar_impl(const char* _Title, ouro::icon_handle _hIcon, const std::function<void()>& _OnStop)
	: OnStop(_OnStop)
	, Percentage(-1)
{
	Controls.fill(nullptr);
	window::init i;
	i.title = _Title;
	i.icon = _hIcon;
	i.action_hook = std::bind(&progress_bar_impl::on_action, this, std::placeholders::_1);
	i.event_hook = std::bind(&progress_bar_impl::on_event, this, std::placeholders::_1);
	i.shape.client_size = int2(320, 106);
	i.shape.state = window_state::hidden;
	i.shape.style = window_style::fixed;
	Window = window::make(i);
	set_percentage_internal(-1);
}

std::shared_ptr<progress_bar> progress_bar::make(const char* _Title, ouro::icon_handle _hIcon, const std::function<void()>& _OnStop)
{
	return std::make_shared<progress_bar_impl>(_Title, _hIcon, _OnStop);
}

void progress_bar_impl::make_controls(const oGUI_EVENT_CREATE_DESC& _CreateEvent)
{
	const int2 ProgressBarSize(270, 22);
	const int2 ButtonSize(75, 23);

	const int2 Inset(10, 10);
	const oRECT rParent = oRect(oWinRectWH(int2(0,0), _CreateEvent.Shape.client_size));

	oGUI_CONTROL_DESC Descs[PB_CONTROL_COUNT];

	// progress/marquee bars
	{
		oRECT rChild = oRect(oWinRectWH(int2(Inset.x, 0), ProgressBarSize));
		oRECT rText = oGUIResolveRect(rParent, rChild, alignment::middle_left, true);
		Descs[PB_MARQUEE].Type = control_type::progressbar_unknown;
		Descs[PB_MARQUEE].Position = oWinRectPosition(oWinRect(rText));
		Descs[PB_MARQUEE].Size = ProgressBarSize;

		Descs[PB_PROGRESS].Type = control_type::progressbar;
		Descs[PB_PROGRESS].Position = oWinRectPosition(oWinRect(rText));
		Descs[PB_PROGRESS].Size = ProgressBarSize;
	}

	// percentage text
	{
		const auto& cpb = Descs[PB_PROGRESS];
		Descs[PB_PERCENT].Type = control_type::label;
		Descs[PB_PERCENT].Text = "0%";
		Descs[PB_PERCENT].Position = int2(cpb.Position.x + cpb.Size.x + 10, cpb.Position.y + 3);
		Descs[PB_PERCENT].Size = int2(35, cpb.Size.y);
	}

	// Stop button
	{
		oRECT rChild = oRect(oWinRectWH(-Inset, ButtonSize));
		oRECT rButton = oGUIResolveRect(rParent, rChild, alignment::bottom_right, true);

		Descs[PB_BUTTON].Type = control_type::button;
		Descs[PB_BUTTON].Text = "&Stop";
		Descs[PB_BUTTON].Position = oWinRectPosition(oWinRect(rButton));
		Descs[PB_BUTTON].Size = oWinRectSize(oWinRect(rButton));
	}

	// text/subtext
	{
		Descs[PB_TEXT].Type = control_type::label_centered;
		Descs[PB_TEXT].Position = Inset;
		Descs[PB_TEXT].Size = int2(_CreateEvent.Shape.client_size.x - 2*Inset.x, 20);

		Descs[PB_SUBTEXT].Type = control_type::label;
		Descs[PB_SUBTEXT].Position = int2(Inset.x, Descs[PB_PROGRESS].Position.y + Descs[PB_PROGRESS].Size.y + 5);
		Descs[PB_SUBTEXT].Size = int2((Descs[PB_PROGRESS].Size.x * 3) / 4, 20);
	}

	for (short i = 0; i < PB_CONTROL_COUNT; i++)
	{
		Descs[i].hParent = _CreateEvent.hWindow;
		Descs[i].ID = i;
		Controls[i] = (ouro::window_handle)oWinControlCreate(Descs[i]);
	}
}

void progress_bar_impl::on_event(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case gui_event::creating:
		{
			make_controls(_Event.AsCreate());
			break;
		}

		default:
			break;
	}
}

void progress_bar_impl::on_action(const oGUI_ACTION_DESC& _Action)
{
	if (_Action.Action == gui_action::control_activated && _Action.DeviceID == PB_BUTTON && OnStop)
	{
		stopped(true);
		OnStop();
	}
}

void progress_bar_impl::stopped(bool _Stopped)
{
	Window->dispatch([=]
	{
		oWinControlSetErrorState(get(PB_PROGRESS), _Stopped);
		oWinControlSetErrorState(get(PB_MARQUEE), _Stopped);
	});
}

void progress_bar_impl::set_textv(const char* _Format, va_list _Args)
{
	lstring s;
	if (-1 == vsnprintf(s, _Format, _Args))
	{
		ellipsize(s);
		oTHROW0(no_buffer_space);
	}
	Window->dispatch([=] { oWinControlSetText(get(PB_TEXT), s.c_str()); });
}

void progress_bar_impl::set_subtextv(const char* _Format, va_list _Args)
{
	lstring s;
	if (-1 == vsnprintf(s, _Format, _Args))
	{
		ellipsize(s);
		oTHROW0(no_buffer_space);
	}
	Window->dispatch([=] { oWinControlSetText(get(PB_SUBTEXT), s.c_str()); });
}

static void ensure_visible(HWND _hControl)
{
	if (!oWinControlIsVisible(_hControl))
		oWinControlSetVisible(_hControl);
}

static void ensure_hidden(HWND _hControl)
{
	if (oWinControlIsVisible(_hControl))
		oWinControlSetVisible(_hControl, false);
}

void progress_bar_impl::set_percentage_internal(HWND _hProgress, HWND _hMarquee, HWND _hPercent, int _Percentage)
{
	if (_Percentage < 0)
	{
		ensure_hidden(_hProgress);
		ensure_hidden(_hPercent);
		ensure_visible(_hMarquee);
	}

	else
	{
		UINT p = (UINT)__max(0, __min(100, (UINT)_Percentage));
		oWinControlSetRangePosition(_hProgress, p);

		char buf[16];
		snprintf(buf, "%u%%", p);
		ellipsize(buf);
		if (!oWinControlSetText(_hPercent, buf))
			throw std::runtime_error("oWinControlSetText failed");

		ensure_hidden(_hMarquee);
		ensure_visible(_hProgress);
		ensure_visible(_hPercent);
	}

	Percentage = _Percentage;
}

} // namespace ouro
