/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#pragma once
#ifndef oCamera_h
#define oCamera_h

#include <oBasis/oInterface.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>

typedef oFUNCTION<void(const oSURFACE_DESC& _Desc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _Mapped)> oCameraOnFrameFn;

interface oCameraFrameStream : oInterface
{
	// A mechanism by which client code can latch into a camera's video feed.

	virtual bool AddObserver(const oCameraOnFrameFn& _OnFrame) = 0;
	virtual bool RemoveObservers() = 0;
};

interface oCameraArticulator : oInterface
{
	// Some cameras provide a motor to control their orientation. This exposed 
	// control for those types that support it.

	virtual bool SetPitch(float _AngleInDegrees) = 0;
	virtual bool GetPitch(float* _pAngleInDegrees) = 0;
};

interface oCameraPosition : oInterface
{
	// Some cameras can provide an estimation of their position based off of calibration or
	// what they see in the frame.  Cameras also have a defined position which is relative to a known
	// location, such as the primary screen.

	virtual bool GetPosition(float3* _pCameraPositionOffset) = 0;
	virtual bool GetEstimatedPosition(float3* _pEstimatedPosition) = 0;
};

interface oCamera : oInterface
{
	struct MODE
	{
		int2 Dimensions;
		oSURFACE_FORMAT Format;
		int BitRate;
	};

	struct DESC
	{
		MODE Mode;
	};

	struct MAPPED
	{
		const void* pData;
		unsigned int RowPitch;
		unsigned int Frame;
	};

	virtual void GetDesc(DESC* _pDesc) threadsafe = 0;

	virtual const char* GetName() const threadsafe = 0;
	virtual unsigned int GetID() const threadsafe = 0;

	virtual bool FindClosestMatchingMode(const MODE& _ModeToMatch, MODE* _pClosestMatch) threadsafe = 0;
	virtual bool GetModeList(unsigned int* _pNumModes, MODE* _pModes) threadsafe = 0;

	virtual float GetFPS() const threadsafe = 0;

	virtual bool SetMode(const MODE& _Mode) threadsafe = 0;

	virtual bool SetCapturing(bool _Capturing = true) threadsafe = 0;
	virtual bool IsCapturing() const threadsafe = 0;

	virtual bool Map(MAPPED* _pMapped) threadsafe = 0;
	virtual void Unmap() threadsafe = 0;
};

// Enumerate all cameras currently attached to the system. If this fails, 
// oErrorGetLast() could be oERROR_NOT_FOUND for 64-bit systems that most likely don't
// have compatible camera drivers, or ENODEV a devices was found on the system,
// but isn't valid. ENOENT means no device was found at all and indicates that
// iteration is finished. When iterating through cameras, it is often useful to 
// skip meaningful work on ENODEV, but not exit out of the iteration loop.
bool oCameraEnum(unsigned int _Index, threadsafe oCamera** _ppCamera);

#endif
