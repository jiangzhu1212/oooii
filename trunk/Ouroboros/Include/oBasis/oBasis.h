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
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oBasis_h
#define oBasis_h
#include <oBasis/oAlgorithm.h>
#include <oBasis/oAllocator.h>
#include <oBasis/oAllocatorTLSF.h>
#include <oBasis/oArcball.h>
#include <oBasis/oAssert.h>
#include <oBasis/oAtof.h>
#include <oBasis/oBackoff.h>
#include <oBasis/oBit.h>
#include <oBasis/oBlockAllocatorFixed.h>
#include <oBasis/oBlockAllocatorGrowable.h>
#include <oBasis/oBuffer.h>
#include <oBasis/oBufferPool.h>
#include <oBasis/oByte.h>
#include <oBasis/oByteSwizzle.h>
#include <oBasis/oCallable.h>
#include <oBasis/oColor.h>
#include <oBasis/oConcurrentIndexAllocator.h>
#include <oBasis/oConditionVariable.h>
#include <oBasis/oCountdownLatch.h>
#include <oBasis/oCppParsing.h>
#include <oBasis/oCSV.h>
#include <oBasis/oDate.h>
#include <oBasis/oDispatchQueue.h>
#include <oBasis/oDispatchQueueConcurrent.h>
#include <oBasis/oDispatchQueueGlobal.h>
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBasis/oEightCC.h>
#include <oBasis/oEqual.h>
#include <oBasis/oError.h>
#include <oBasis/oEvent.h>
#include <oBasis/oEye.h>
#include <oBasis/oFilterChain.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oFor.h>
#include <oBasis/oFourCC.h>
#include <oBasis/oFunction.h>
#include <oBasis/oGeometry.h>
#include <oBasis/oGUI.h>
#include <oBasis/oGUID.h>
#include <oBasis/oHash.h>
#include <oBasis/oHLSLBit.h>
#include <oBasis/oHLSLMath.h>
#include <oBasis/oHLSLStructs.h>
#include <oBasis/oHLSLTypes.h>
#include <oBasis/oIndexAllocator.h>
#include <oBasis/oINI.h>
#include <oBasis/oInt.h>
#include <oBasis/oInterface.h>
#include <oBasis/oIntrinsic.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oJoinable.h>
#include <oBasis/oLimits.h>
#include <oBasis/oLinearAllocator.h>
#include <oBasis/oLockedPointer.h>
#include <oBasis/oLockedVector.h>
#include <oBasis/oLockFreeQueue.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMath.h>
#include <oBasis/oMemory.h>
#include <oBasis/oMeshUtil.h>
#include <oBasis/oMIME.h>
#include <oBasis/oMutex.h>
#include <oBasis/oNoncopyable.h>
#include <oBasis/oOBJ.h>
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oOperators.h>
#include <oBasis/oOSC.h>
#include <oBasis/oPath.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oStdAllocator.h>
#include <oBasis/oStdAtomic.h>
#include <oBasis/oStdChrono.h>
#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oStddef.h>
#include <oBasis/oStdFuture.h>
#include <oBasis/oStdMutex.h>
#include <oBasis/oStdRatio.h>
#include <oBasis/oStdThread.h>
#include <oBasis/oStdTypeTraits.h>
#include <oBasis/oString.h>
#include <oBasis/oStringize.h>
#include <oBasis/oSurface.h>
#include <oBasis/oSurfaceFill.h>
#include <oBasis/oTask.h>
#include <oBasis/oThread.h>
#include <oBasis/oThreadsafe.h>
#include <oBasis/oTimer.h>
#include <oBasis/oTypes.h>
#include <oBasis/oUint128.h>
#include <oBasis/oUnorderedMap.h>
#include <oBasis/oURI.h>
#include <oBasis/oVersion.h>
#include <oBasis/oX11KeyboardSymbols.h>
#include <oBasis/oXML.h>
#endif
