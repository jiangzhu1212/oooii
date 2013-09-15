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
// Interface for working with monitors/displays
#pragma once
#ifndef oCore_module_h
#define oCore_module_h

#include <oStd/version.h>

namespace oCore {
	namespace module {

/* enum class */ namespace type
{	enum value {

	unknown,
	app,
	dll,
	lib,
	font_unknown,
	font_raster,
	font_truetype,
	font_vector,
	virtual_device,
	drv_unknown,
	drv_comm,
	drv_display,
	drv_installable,
	drv_keyboard,
	drv_language,
	drv_mouse,
	drv_network,
	drv_printer,
	drv_sound,
	drv_system,

};}

class id
{
public:
	id() : Handle(nullptr) {}

	bool operator==(const id& _That) const { return Handle == _That.Handle; }
	bool operator!=(const id& _That) const { return !(*this == _That); }
	operator bool() const { return !!Handle; }

private:
	void* Handle;
};

struct info
{
	info()
		: type(type::unknown)
		, is_debug(false)
		, is_prerelease(false)
		, is_patched(false)
		, is_private(false)
		, is_special(false)
	{}

	struct oStd::version version;
	oStd::mstring company;
	oStd::mstring description;
	oStd::mstring product_name;
	oStd::mstring copyright;
	oStd::mstring original_filename;
	oStd::mstring comments;
	oStd::mstring private_message;
	oStd::mstring special_message;
	type::value type;
	bool is_debug;
	bool is_prerelease;
	bool is_patched;
	bool is_private;
	bool is_special;
};

// similar to dlopen, et al.
id open(const oStd::path& _Path);
void close(id _ModuleID);
void* sym(id _ModuleID, const char* _SymbolName);

oStd::path get_path(id _ModuleID);

// given an address in a module, retreive that module's handle
// fit for name(), sym() or close() APIs.
id get_id(void* _Symbol);

info get_info(const oStd::path& _Path);
info get_info(id _ModuleID);

	} // namespace module

	namespace this_module {

oStd::path path();
module::info get_info();

	} // namespace this_module
} // namespace oCore

#endif
