// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oCore_win_version_h
#define oCore_win_version_h

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ouro {
	namespace windows {

/* enum class */ namespace version
{	enum value {

	unknown,
	win2000,
	xp,
	xp_pro_64bit,
	server_2003,
	home_server,
	server_2003r2,
	vista,
	server_2008,
	vista_sp1,
	server_2008_sp1,
	vista_sp2,
	server_2008_sp2,
	win7,
	server_2008r2,
	win7_sp1,
	server_2008r2_sp1,
	win8,
	server_2012,
	win8_1,
	server_2012_sp1,

};}

version::value get_version();

	} // namespace windows
} // namespace ouro

#endif
