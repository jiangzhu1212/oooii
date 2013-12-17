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
#include <oGUI/about.h>
#include <oGUI/window.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/module.h>

static void show_modal(const std::shared_ptr<ouro::window>& _Parent, const std::shared_ptr<ouro::basic_window>& _Child, bool _Show)
{
	_Parent->enabled(!_Show);
	_Child->owner(_Show ? _Parent : nullptr);
	_Child->show(_Show ? oGUI_WINDOW_RESTORED : oGUI_WINDOW_HIDDEN);
}

namespace ouro {

class about_impl : public about
{
public:
	about_impl(const info& _Info);

	// basic_window API
	oGUI_WINDOW native_handle() const override { return Window->native_handle(); }
	display::id display_id() const override { return Window->display_id(); }
	bool is_window_thread() const override { return Window->is_window_thread(); }
	void flush_messages(bool _WaitForNext = false) override { Window->flush_messages(_WaitForNext); }
	void quit() override { Window->quit(); }
	void debug(bool _Debug) override { Window->debug(_Debug); }
	bool debug() const override { return Window->debug(); }
	void state(oGUI_WINDOW_STATE _State) override { Window->state(_State); }
	oGUI_WINDOW_STATE state() const override { return Window->state(); }
	void client_position(const int2& _ClientPosition) override { Window->client_position(_ClientPosition); }
	int2 client_position() const override { return Window->client_position(); }
	int2 client_size() const override { return Window->client_size(); }
	void icon(oGUI_ICON _hIcon) override { Window->icon(_hIcon); }
	oGUI_ICON icon() const override { return Window->icon(); }
	void user_cursor(oGUI_CURSOR _hCursor) override { Window->user_cursor(_hCursor); }
	oGUI_CURSOR user_cursor() const override { return Window->user_cursor(); }
	void client_cursor_state(oGUI_CURSOR_STATE _State) override { Window->client_cursor_state(_State); }
	oGUI_CURSOR_STATE client_cursor_state() const override { return Window->client_cursor_state(); }
	void set_titlev(const char* _Format, va_list _Args) override { Window->set_titlev(_Format, _Args); }
	char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const override { return Window->get_title(_StrDestination, _SizeofStrDestination); }
	void parent(const std::shared_ptr<basic_window>& _Parent) override { Window->parent(_Parent); }
	std::shared_ptr<basic_window> parent() const override { return Window->parent(); }
	void owner(const std::shared_ptr<basic_window>& _Owner) override { Window->owner(_Owner); }
	std::shared_ptr<basic_window> owner() const override { return Window->owner(); }
	void sort_order(oGUI_WINDOW_SORT_ORDER _SortOrder) override { Window->sort_order(_SortOrder); }
	oGUI_WINDOW_SORT_ORDER sort_order() const override { return Window->sort_order(); }
	void focus(bool _Focus) override { Window->focus(_Focus); }
	bool has_focus() const override { return Window->has_focus(); }

	// about API
	void show_modal(const std::shared_ptr<window>& _Parent) override;
	
private:
	std::shared_ptr<window> Window;
	std::shared_ptr<window> Parent;
	std::vector<std::string> ComponentComments;

	enum AB_CONTROL
	{
		AB_ICON,
		AB_INSTALL_PATH,
		AB_OK,
		AB_MODULE_INFO,
		AB_WEBSITE,
		AB_ISSUE_SITE,
		AB_COMPONENT_GROUP,
		AB_COMPONENT_LIST,
		AB_COMPONENT_COMMENT,
		AB_CONTROL_COUNT,
	};

	std::array<oGUI_WINDOW, AB_CONTROL_COUNT> Controls;

private:
	
	struct init
	{
		init(const info& _Info, const module::info& _ModuleInfo)
			: info(_Info)
			, module_info(_ModuleInfo)
		{}

		const struct info& info;
		const module::info& module_info;
	};

	void on_event(const oGUI_EVENT_DESC& _Event);
	void on_action(const oGUI_ACTION_DESC& _Action);
	bool make_controls(const oGUI_EVENT_CREATE_DESC& _CreateEvent);
};

static const int kComponentHeight = 100;

about_impl::about_impl(const info& _Info)
{
	module::info mi = this_module::get_info();

	init Init(_Info, mi);

	sstring Title;
	snprintf(Title, "About %s", mi.product_name.c_str());

	Controls.fill(nullptr);
	
	window::init i;
	i.title = Title;
	i.icon = nullptr;
	i.create_user_data = &Init;
	i.action_hook = std::bind(&about_impl::on_action, this, std::placeholders::_1);
	i.event_hook = std::bind(&about_impl::on_event, this, std::placeholders::_1);
	i.shape.State = oGUI_WINDOW_HIDDEN;
	i.shape.Style = oGUI_WINDOW_DIALOG;
	i.shape.ClientSize = int2(320, 140);

	if (oSTRVALID(_Info.components))
		i.shape.ClientSize.y += kComponentHeight;
	
	Window = window::make(i);
}

std::shared_ptr<about> about::make(const info& _Info)
{
	return std::make_shared<about_impl>(_Info);
}

bool about_impl::make_controls(const oGUI_EVENT_CREATE_DESC& _CreateEvent)
{
	const init& Init = *(const init*)_CreateEvent.pUser;
	const info& Info = Init.info;
	const module::info& mi = Init.module_info;

	const int2 kInset(10, 10);
	
	const int2 kIconSize(75, 75);
	const int2 kIconPos(0, 0);

	const int2 kModulePathSize(_CreateEvent.Shape.ClientSize.x - kInset.x * 2, 23);
	const int2 kModulePathPos(kInset.x, kIconPos.y + kIconSize.y);
	
	const int2 kModuleInfoSize(220, 40);
	const int2 kModuleInfoPos(kIconPos.x + kIconSize.x, kInset.y);
	
	const int2 kWebsiteSize(75, 23);
	const int2 kWebsitePos(kModuleInfoPos.x, kModuleInfoPos.y + kModuleInfoSize.y);

	const int2 kIssueSiteSize(75, 23);
	const int2 kIssueSitePos(_CreateEvent.Shape.ClientSize.x - kIssueSiteSize.x - kInset.x, kWebsitePos.y);

	const int2 kComponentGroupSize(_CreateEvent.Shape.ClientSize.x - kInset.x * 2, kComponentHeight);
	const int2 kComponentGroupPos(kInset.x, kIconPos.y + kIconSize.y + 20);

	const int2 kComponentListSize(90, kComponentGroupSize.y - 21);
	const int2 kComponentListPos(kComponentGroupPos.x + 5, kComponentGroupPos.y + 15);

	const int2 kComponentCommentSize(kComponentGroupSize.x - kComponentListSize.x - 15, kComponentListSize.y);
	const int2 kComponentCommentPos(kComponentListPos.x + kComponentListSize.x + 5, kComponentListPos.y);

	const int2 kButtonSize(75, 23);

	const oRECT rParent = oRect(oWinRectWH(int2(0,0), _CreateEvent.Shape.ClientSize));
	
	oGUI_CONTROL_DESC Descs[AB_CONTROL_COUNT];

	// Icon
	{
		oRECT rChild = oRect(oWinRectWH(int2(0,0), kIconSize));
		oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

		Descs[AB_ICON].Type = oGUI_CONTROL_ICON;
		Descs[AB_ICON].Icon = Info.icon;
		Descs[AB_ICON].Position = oWinRectPosition(oWinRect(r));
		Descs[AB_ICON].Size = oWinRectSize(oWinRect(r));
	}

	// Install Path
	lstring StrModulePath;
	{
		path ModulePath = this_module::get_path();

		snprintf(StrModulePath, "Install Path: <a href=\"file://%s\">%s</a>", ModulePath.parent_path().c_str(), ModulePath.c_str());

		oRECT rChild = oRect(oWinRectWH(kModulePathPos, kModulePathSize));
		oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

		Descs[AB_INSTALL_PATH].Type = oGUI_CONTROL_HYPERLABEL;
		Descs[AB_INSTALL_PATH].Text = StrModulePath;
		Descs[AB_INSTALL_PATH].Position = oWinRectPosition(oWinRect(r));
		Descs[AB_INSTALL_PATH].Size = oWinRectSize(oWinRect(r));
	}

	// Module Info
	xlstring StrModuleInfo;
	{
		sstring StrVersion;
		snprintf(StrModuleInfo, "%s%s%s\n%s%s%s%s%s\n%s"
			, mi.product_name.c_str()
			, mi.is_64bit_binary ? "" : " 32-bit"
			, mi.is_debug ? " debug" : ""
			, to_string(StrVersion, mi.version)
			, mi.is_prerelease ? " pre-release" : ""
			, mi.is_patched ? " patched" : ""
			, mi.is_private ? " private" : ""
			, mi.is_special ? " special" : ""
			, mi.copyright.c_str());

		oRECT rChild = oRect(oWinRectWH(kModuleInfoPos, kModuleInfoSize));
		oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

		Descs[AB_MODULE_INFO].Type = oGUI_CONTROL_LABEL;
		Descs[AB_MODULE_INFO].Text = StrModuleInfo;
		Descs[AB_MODULE_INFO].Position = oWinRectPosition(oWinRect(r));
		Descs[AB_MODULE_INFO].Size = oWinRectSize(oWinRect(r));

		//struct version version;
		//mstring company;
		//mstring description;
		//mstring product_name;
		//mstring copyright;
		//mstring original_filename;
		//mstring comments;
		//mstring private_message;
		//mstring special_message;
	}

	// Website
	lstring StrWebsite;
	{
		if (oSTRVALID(Info.website))
		{
			snprintf(StrWebsite, "<a href=\"%s\">visit our website</a>", Info.website);

			oRECT rChild = oRect(oWinRectWH(kWebsitePos, kWebsiteSize));
			oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

			Descs[AB_WEBSITE].Type = oGUI_CONTROL_HYPERLABEL;
			Descs[AB_WEBSITE].Text = StrWebsite;
			Descs[AB_WEBSITE].Position = oWinRectPosition(oWinRect(r));
			Descs[AB_WEBSITE].Size = oWinRectSize(oWinRect(r));
		}
	}

	// Issue Site
	lstring StrIssueSite;
	{
		if (oSTRVALID(Info.issue_site))
		{
			snprintf(StrIssueSite, "<a href=\"%s\">report an issue</a>", Info.issue_site);

			oRECT rChild = oRect(oWinRectWH(kIssueSitePos, kIssueSiteSize));
			oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

			Descs[AB_ISSUE_SITE].Type = oGUI_CONTROL_HYPERLABEL;
			Descs[AB_ISSUE_SITE].Text = StrIssueSite;
			Descs[AB_ISSUE_SITE].Position = oWinRectPosition(oWinRect(r));
			Descs[AB_ISSUE_SITE].Size = oWinRectSize(oWinRect(r));
		}
	}

	// OK Button
	{
		oRECT rChild = oRect(oWinRectWH(-kInset, kButtonSize));
		oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_BOTTOM_RIGHT, true);

		Descs[AB_OK].Type = oGUI_CONTROL_BUTTON;
		Descs[AB_OK].Text = "&OK";
		Descs[AB_OK].Position = oWinRectPosition(oWinRect(r));
		Descs[AB_OK].Size = oWinRectSize(oWinRect(r));
	}

	// Component Group
	{
		if (oSTRVALID(Info.components))
		{
			oRECT rChild = oRect(oWinRectWH(kComponentGroupPos, kComponentGroupSize));
			oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

			Descs[AB_COMPONENT_GROUP].Type = oGUI_CONTROL_GROUPBOX;
			Descs[AB_COMPONENT_GROUP].Text = "3rd-Party Components";
			Descs[AB_COMPONENT_GROUP].Position = oWinRectPosition(oWinRect(r));
			Descs[AB_COMPONENT_GROUP].Size = oWinRectSize(oWinRect(r));
		}
	}
	
	// Component List
	{
		if (oSTRVALID(Info.components))
		{
			oRECT rChild = oRect(oWinRectWH(kComponentListPos, kComponentListSize));
			oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

			Descs[AB_COMPONENT_LIST].Type = oGUI_CONTROL_LISTBOX;
			Descs[AB_COMPONENT_LIST].Text = Info.components;
			Descs[AB_COMPONENT_LIST].Position = oWinRectPosition(oWinRect(r));
			Descs[AB_COMPONENT_LIST].Size = oWinRectSize(oWinRect(r));

			tokenize(ComponentComments, Info.component_comments, "|");
		}
	}

	// Component Comment
	{
		if (oSTRVALID(Info.components))
		{
			oRECT rChild = oRect(oWinRectWH(kComponentCommentPos, kComponentCommentSize));
			oRECT r = oGUIResolveRect(rParent, rChild, oGUI_ALIGNMENT_TOP_LEFT, true);

			Descs[AB_COMPONENT_COMMENT].Type = oGUI_CONTROL_HYPERLABEL;
			Descs[AB_COMPONENT_COMMENT].Text = ComponentComments[0].c_str();
			Descs[AB_COMPONENT_COMMENT].Position = oWinRectPosition(oWinRect(r));
			Descs[AB_COMPONENT_COMMENT].Size = oWinRectSize(oWinRect(r));
		}
	}

	for (short i = 0; i < AB_CONTROL_COUNT; i++)
	{
		if (Descs[i].Type != oGUI_CONTROL_UNKNOWN)
		{
			Descs[i].hParent = _CreateEvent.hWindow;
			Descs[i].ID = i;
			Controls[i] = (oGUI_WINDOW)oWinControlCreate(Descs[i]);
		}
		else
			Controls[i] = nullptr;
	}

	return true;
}

void about_impl::on_event(const oGUI_EVENT_DESC& _Event)
{
	switch (_Event.Type)
	{
		case oGUI_CREATING:
		{
			if (!make_controls(_Event.AsCreate()))
				oThrowLastError();
			break;
		}

		default:
			break;
	}
}

void about_impl::on_action(const oGUI_ACTION_DESC& _Action)
{
	switch (_Action.Action)
	{
		case oGUI_ACTION_CONTROL_ACTIVATED:
			switch (_Action.DeviceID)
			{
				case AB_OK:
				{
					::show_modal(Parent, Window, false);
					Parent = nullptr;
					quit();
					break;
				}

				case AB_COMPONENT_LIST:
				{
					int index = oWinControlGetSelectedSubItem((HWND)_Action.hWindow);
					if (index >= 0)
					{
						const char* comment = "";
						if (index < static_cast<int>(ComponentComments.size()))
							comment = ComponentComments[index].c_str();
						oWinControlSetText((HWND)Controls[AB_COMPONENT_COMMENT], comment);
					}
					break;
				}

				default:
					break;
			}
			break;

		default:
			break;
	}
}

void about_impl::show_modal(const std::shared_ptr<window>& _Parent)
{
	Parent = _Parent;
	Window->icon(Parent->icon());
	::show_modal(Parent, Window, true);
	Window->flush_messages(true);
	Window->icon(nullptr);
}

} // namespace ouro
