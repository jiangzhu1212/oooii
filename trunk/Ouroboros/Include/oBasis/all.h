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
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oBasis_all_h
#define oBasis_all_h
#include <oBasis/oAirKeyboard.h>
#include <oBasis/oAllocator.h>
#include <oBasis/oAllocatorTLSF.h>
#include <oBasis/oArcball.h>
#include <oBasis/oBasisRequirements.h>
#include <oBasis/oBuffer.h>
#include <oBasis/oBufferPool.h>
#include <oBasis/oCameraController.h>
#include <oBasis/oCameraControllerArcball.h>
#include <oBasis/oCameraControllerMaya.h>
#include <oBasis/oCameraControllerMMO.h>
#include <oBasis/oCompression.h>
#include <oBasis/oContainer.h>
#include <oBasis/oCppParsing.h>
#include <oBasis/oDispatchQueue.h>
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBasis/oDispatchQueueGlobal.h>
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBasis/oEightCC.h>
#include <oBasis/oError.h>
#include <oBasis/oEye.h>
#include <oBasis/oFilterChain.h>
#include <oBasis/oGeometry.h>
#include <oBasis/oGUI.h>
#include <oBasis/oGZip.h>
#include <oBasis/oInputMapper.h>
#include <oBasis/oInt.h>
#include <oBasis/oInterface.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oLockedPointer.h>
#include <oBasis/oLZMA.h>
#include <oBasis/oMath.h>
#include <oBasis/oMemory.h>
#include <oBasis/oMeshUtil.h>
#include <oBasis/oMIME.h>
#include <oBasis/oOBJ.h>
#include <oBasis/oOSC.h>
#include <oBasis/oPath.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oResizedType.h>
#include <oBasis/oSnappy.h>
#include <oBasis/oStddef.h>
#include <oBasis/oStdStringSupport.h>
#include <oBasis/oString.h>
#include <oBasis/oSurface.h>
#include <oBasis/oSurfaceFill.h>
#include <oBasis/oSurfaceResize.h>
#include <oBasis/oTimer.h>
#include <oBasis/oTypeID.h>
#include <oBasis/oTypeInfo.h>
#include <oBasis/oTypes.h>
#include <oBasis/oURI.h>
#include <oBasis/oVersion.h>
#endif
