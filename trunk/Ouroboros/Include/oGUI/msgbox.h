// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Printf-style interface for displaying platform message boxes
#pragma once
#ifndef oGUI_msgbox_h
#define oGUI_msgbox_h

#include <oGUI/oGUI.h>
#include <stdarg.h>

namespace ouro {

  namespace msg_type
  { enum value {
  
    info,
    warn,
    error,
    debug,
    yesno,
    notify,
    notify_info,
    notify_warn,
    notify_error,

  };}
  
  namespace msg_result
  { enum value {

    no,
    yes,
    abort,
    debug,
    ignore,
    ignore_always,

  };}

	msg_result::value msgboxv(msg_type::value _Type, ouro::window_handle _hParent, const char* _Title, const char* _Format, va_list _Args);
  inline msg_result::value msgbox(msg_type::value _Type, ouro::window_handle _hParent, const char* _Title, const char* _Format, ...) { va_list args; va_start(args, _Format); msg_result::value r = msgboxv(_Type, _hParent, _Title, _Format, args); va_end(args); return r; }

}

#endif
