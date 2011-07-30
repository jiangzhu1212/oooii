/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// A simple progress bar for very simple applications. NOTE: This is for the 90%
// case. Dialogs are fairly easy to construct and should be tailored to the task,
// so if there's a complex case such as wanting to see individual task v. overall
// progress, or seeing progress of each thread, a different dialog box should be
// created. In the meantime, ALL components of a major task should add up to the
// single completion of the progress bar. I.e. don't have Step1 go to 100% and
// then Step2 resets and goes to 100%, just update the task and have each add up
// to 50% of the overall progress. See GUI suggestions on the net. There's a 
// good starting point at http://msdn.microsoft.com/en-us/library/aa511486.aspx
#pragma once
#ifndef oProgressBar_h
#define oProgressBar_h

#include <oooii/oStddef.h>

void oProgressBarShow(const char* _Title, bool _Show = true, bool _AlwaysOnTop = true, bool _UnknownProgress = false);
void oProgressBarSetText(const char* _Text, const char* _Subtext = nullptr);
void oProgressBarSetStopped(bool _Stopped = true);
void oProgressBarSet(int _Percentage);
void oProgressBarAdd(int _Percentage);
bool oProgressBarWait(unsigned int _TimeoutMS = oINFINITE_WAIT);

#endif
