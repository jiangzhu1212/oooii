// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oCore_win_com_h
#define oCore_win_com_h

namespace ouro {
	namespace windows {
		namespace com {

// Needs to be call per-thread that uses COM
void ensure_initialized();
		
		} // namespace com
	} // namespace windows
} // namespace ouro

#endif
