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
// This include-all-headers-in-oooii.lib header is intended
// for use only in the precompiled headers of dependent libs.
// Using this as a short-cut for includes can cause precompiled-
// header-related build dependency problems, so still be 
// disciplined in the use of specific and minimal include files.
#pragma once
#ifndef oooii_h
#define oooii_h
#include <oPlatform/oConsole.h>
#include <oPlatform/oCPU.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oInterprocessEvent.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oMirroredArena.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oP4.h>
#include <oPlatform/oPageAllocator.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oProgressBar.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oSocket.h>
#include <oPlatform/oStream.h>
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oTest.h>
#include <oPlatform/oThreadX.h>
#include <oPlatform/oWindow.h>
#endif
