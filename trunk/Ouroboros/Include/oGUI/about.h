/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// A simple about dialog that gets most of its information from the module info.
#pragma once
#ifndef oGUI_about_h
#define oGUI_about_h

#include <oGUI/window.h>

namespace ouro {

class about : public basic_window
{
public:
	struct info
	{
		info()
			: icon(nullptr)
			, website(nullptr)
			, issue_site(nullptr)
			, components(nullptr)
			, component_comments(nullptr)
		{}

		// Icon used to represent the app in the about dialog
		ouro::icon_handle icon;

		// A URL that will be clickable in the about dialog under "visit our website"
		const char* website;

		// A URL that will be clickable in the about dialog under "report an issue"
		const char* issue_site;

		// A series of strings that are listed in a listbox, usually libraries or 
		// plugins used by the app. Each should be delimited by '|'. For example:
		// "zlib|libpng|tbb"
		const char* components;

		// A more verbose description is displayed for the selected component, so 
		// this list must be accessible by the same index as the above components.
		const char* component_comments;
	};

	static std::shared_ptr<about> make(const info& _Info);

	virtual void show_modal(const std::shared_ptr<window>& _Parent) = 0;
};

} // namespace ouro

#endif
