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
#pragma once
#ifndef oCamera_h
#define oCamera_h

#include <oBasis/oInterface.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>

typedef oFUNCTION<void(int _ID, const oSURFACE_DESC& _Desc, const oSURFACE_CONST_MAPPED_SUBRESOURCE& _Mapped)> oCAMERA_FRAME_HOOK;

// {B7DFAE55-8AA4-41DB-82E2-FC498DCC78E4}
oDEFINE_GUID_I(oCameraFrameStream, 0xb7dfae55, 0x8aa4, 0x41db, 0x82, 0xe2, 0xfc, 0x49, 0x8d, 0xcc, 0x78, 0xe4);
interface oCameraFrameStream : oInterface
{
	// A mechanism by which client code can latch into a camera's video feed.

	virtual bool AddObserver(const oCAMERA_FRAME_HOOK& _OnFrame) = 0;
	virtual bool RemoveObservers() = 0;
};

// {343AD92B-1BE1-4EDD-A810-34FAE2877CBD}
oDEFINE_GUID_I(oCameraArticulator, 0x343ad92b, 0x1be1, 0x4edd, 0xa8, 0x10, 0x34, 0xfa, 0xe2, 0x87, 0x7c, 0xbd);
interface oCameraArticulator : oInterface
{
	// Some cameras provide a motor to control their orientation. This exposed 
	// control for those types that support it.

	virtual bool SetPitch(float _AngleInDegrees) = 0;
	virtual bool GetPitch(float* _pAngleInDegrees) = 0;
};

// {CFCDA794-3E4D-4841-9C5A-03CDAD691FD3}
oDEFINE_GUID_I(oCameraPosition, 0xcfcda794, 0x3e4d, 0x4841, 0x9c, 0x5a, 0x3, 0xcd, 0xad, 0x69, 0x1f, 0xd3);
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
// check oErrorGetLast() for more details. If there is a failure it is often
// because the installed drivers are not up-to-date.
bool oCameraEnum(unsigned int _Index, threadsafe oCamera** _ppCamera);

#endif
