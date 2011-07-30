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
#include <oooii/oAllocator.h>
#include <oooii/oAllocatorTLSF.h>
#include <oooii/oAssert.h>
#include <oooii/oAsyncFileIO.h>
#include <oooii/oAtof.h>
#include <oooii/oAtomic.h>
#include <oooii/oBit.h>
#include <oooii/oByte.h>
#include <oooii/oBuffer.h>
#include <oooii/oBufferPool.h>
#include <oooii/oCheckedAssignment.h>
#include <oooii/oColor.h>
#include <oooii/oConcurrentIndexAllocator.h>
#include <oooii/oConcurrentPooledAllocator.h>
#include <oooii/oConcurrentQueueMS.h>
#include <oooii/oConcurrentQueueOptimisticFIFO.h>
#include <oooii/oConsole.h>
#include <oooii/oCountdownLatch.h>
#include <oooii/oCPU.h>
#include <oooii/oDebugger.h>
#include <oooii/oDescUtil.h>
#include <oooii/oDisplay.h>
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oFile.h>
#include <oooii/oFilterChain.h>
#include <oooii/oGeometry.h>
#include <oooii/oGPU.h>
#include <oooii/oGUID.h>
#include <oooii/oHash.h>
#include <oooii/oHeap.h>
#include <oooii/oImage.h>
#include <oooii/oIndex.h>
#include <oooii/oIndexAllocator.h>
#include <oooii/oINI.h>
#include <oooii/oInterface.h>
#include <oooii/oIntrinsic.h>
#include <oooii/oKeyboard.h>
#include <oooii/oLimits.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oLockedVector.h>
#include <oooii/oLockFreeQueue.h>
#include <oooii/oLockFreeRingBuffer.h>
#include <oooii/oMath.h>
#include <oooii/oMemory.h>
#include <oooii/oMirroredArena.h>
#include <oooii/oModule.h>
#include <oooii/oMouse.h>
#include <oooii/oMsgBox.h>
#include <oooii/oMutex.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oP4.h>
#include <oooii/oPageAllocator.h>
#include <oooii/oPath.h>
#include <oooii/oPooledAllocator.h>
#include <oooii/oProcess.h>
#include <oooii/oProcessHeap.h>
#include <oooii/oProgressBar.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSize.h>
#include <oooii/oSocket.h>
#include <oooii/oStddef.h>
#include <oooii/oStdio.h>
#include <oooii/oStdAlloc.h>
#include <oooii/oSTL.h>
#include <oooii/oString.h>
#include <oooii/oStringID.h>
#include <oooii/oSurface.h>
#include <oooii/oSwizzle.h>
#include <oooii/oTest.h>
#include <oooii/oTimerHelpers.h>
#include <oooii/oThread.h>
#include <oooii/oThreading.h>
#include <oooii/oThreadpool.h>
#include <oooii/oType.h>
#include <oooii/oURI.h>
#include <oooii/oWindow.h>
#include <oooii/oXML.h>
#endif
