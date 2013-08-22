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
// Ubiquitous types often found in header/interface definitions are added here
// to keep includes simple. Only add things here that are used VERY often.
#pragma once
#ifndef oStddef_h
#define oStddef_h

#include <oBasis/oInt.h>
#include <oBasis/oInterface.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oPlatformFeatures.h>
#include <oConcurrency/thread_safe.h>
#include <oStd/fixed_string.h>
#include <oStd/macros.h>

#include <oStd/oStdThread.h>
// the standard is a bit too obtuse for sleeping a thread, so wrap it
inline void oSleep(unsigned int _Milliseconds) { oStd::this_thread::sleep_for(oStd::chrono::milliseconds(_Milliseconds)); }

#endif
