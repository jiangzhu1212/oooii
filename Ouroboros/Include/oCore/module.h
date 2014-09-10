// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Interface for working with monitors/displays
#pragma once
#ifndef oCore_module_h
#define oCore_module_h

#include <oString/path.h>
#include <oBase/version.h>

namespace ouro {
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
		, is_64bit_binary(false)
		, is_debug(false)
		, is_prerelease(false)
		, is_patched(false)
		, is_private(false)
		, is_special(false)
	{}

	struct version version;
	mstring company;
	mstring description;
	mstring product_name;
	mstring copyright;
	mstring original_filename;
	mstring comments;
	mstring private_message;
	mstring special_message;
	type::value type;
	bool is_64bit_binary;
	bool is_debug;
	bool is_prerelease;
	bool is_patched;
	bool is_private;
	bool is_special;
};

// similar to dlopen, et al.
id open(const path& _Path);
void close(id _ModuleID);
void* sym(id _ModuleID, const char* _SymbolName);

// convenience function that given an array of function names will fill an array 
// of the same size with the resolved symbols. If a class is defined that has a 
// run of members defined for the functions, then the address of the first 
// function can be passed as the array to quickly fill an interface of typed
// functions ready for use.
void link(id _ModuleID, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces);
template<size_t size> void link(id _ModuleID, const char* (&_pInterfaceFunctionNames)[size], void** _ppInterfaces) { link(_ModuleID, _pInterfaceFunctionNames, _ppInterfaces, size); }

inline id link(const path& _Path, const char** _pInterfaceFunctionNames, void** _ppInterfaces, size_t _CountofInterfaces) { id ID = open(_Path); link(ID, _pInterfaceFunctionNames, _ppInterfaces, _CountofInterfaces); return ID; }
template<size_t size> id link(const path& _Path, const char* (&_InterfaceFunctionNames)[size], void** _ppInterfaces) { return link(_Path, _InterfaceFunctionNames, _ppInterfaces, size); }

path get_path(id _ModuleID);

// given an address in a module, retreive that module's handle
// fit for name(), sym() or close() APIs.
id get_id(const void* _Symbol);

info get_info(const path& _Path);
info get_info(id _ModuleID);

	} // namespace module

	namespace this_module {

module::id get_id();
path get_path();
module::info get_info();

	} // namespace this_module
} // namespace ouro

#endif
