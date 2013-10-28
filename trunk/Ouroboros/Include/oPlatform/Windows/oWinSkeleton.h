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
// This provides an API similar to that for touch-based devices to abstract 
// skeleton-tracking input devices such as Kinect.
#pragma once
#ifndef oWinSkeleton_h
#define oWinSkeleton_h

#include <oBase/macros.h>
#include <oBasis/oFunction.h>

// Handle used in window messages to identify a particular skeleton
oDECLARE_HANDLE(HSKELETON);

// Use these to register a skeleton source, i.e. Kinect.
// Basically all that's needed is an indirect to call into the integration code
// to get a snapshot of the skeleton.
void oWinRegisterSkeletonSource(HSKELETON _hSkeleton, const oFUNCTION<void(oGUI_BONE_DESC* _pSkeleton)>& _GetSkeleton);
void oWinUnregisterSkeletonSource(HSKELETON _hSkeleton);

// Call this from an oWM_SKELETON message. Returns false if the results in 
// _pSkeleton are not valid.
bool oWinGetSkeletonDesc(HSKELETON _hSkeleton, oGUI_BONE_DESC* _pSkeleton);

#endif
