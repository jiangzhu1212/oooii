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

} // namespace ouro

#endif
