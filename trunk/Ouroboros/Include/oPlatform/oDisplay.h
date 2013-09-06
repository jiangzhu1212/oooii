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
// Interface for working with monitors/displays
#pragma once
#ifndef oDisplay_h
#define oDisplay_h

#include <oBasis/oGPUConcepts.h>

struct oDISPLAY_ADAPTER_DRIVER_DESC
{
	oStd::mstring Description;
	oStd::mstring PlugNPlayID;
	oStd::version Version;
	oGPU_VENDOR Vendor;
};

struct oDISPLAY_MODE
{
	oDISPLAY_MODE()
		: Size(oDEFAULT, oDEFAULT)
		, Bitdepth(oDEFAULT)
		, RefreshRate(oDEFAULT)
	{}
	int2 Size;
		
	// usually 16- or 32-bit
	// oDEFAULT implies current settings
	int Bitdepth;

	// Usually 60, 75, 85, 120, 240
	// oDEFAULT implies current settings
	int RefreshRate;
};

struct oDISPLAY_DESC
{
	oDISPLAY_DESC()
		: NativeHandle(nullptr)
		, Position(oDEFAULT, oDEFAULT)
		, WorkareaPosition(oDEFAULT, oDEFAULT)
		, WorkareaSize(oDEFAULT, oDEFAULT)
		, Index(oInvalid)
		, IsPrimary(false)
		, IsPowerOn(false)
	{}
	
	oDISPLAY_MODE Mode;
	void* NativeHandle; // HMONITOR on Windows
	int2 Position;
	int2 WorkareaPosition;
	int2 WorkareaSize;
	int Index;
	bool IsPrimary;
	bool IsPowerOn;
};

// Walks the display adapters (video cards) currently recognized by the system.
// The enumerator should return true to continue, or false to short-circuit
// enumeration.
void oDisplayAdapterEnum(const oFUNCTION<bool(int _AdapterIndex, const oDISPLAY_ADAPTER_DRIVER_DESC& _DriverDesc)>& _Enumerator);

// Ouroboros features are often dependent on at least a version of a driver, or
// our internal QA has confirmed some very bad behavior in versions of the 
// driver that pre-date the ones returned by this. If this returns an invalid
// version it means Ouroboros has not been extensively tested with that vendor's
// adapter.
oStd::version oDisplayAdapterGetMinimumVersion(oGPU_VENDOR _Vendor);

// Checks that all adapters are up-to-date.
bool oDisplayAdapterIsUpToDate();

// If _Index doesn't exist, this function will return false with an 
// oErrorGetLast() of std::errc::no_such_device. If successful, _pDesc is filled 
// with a description of the specified display.
bool oDisplayEnum(int _Index, oDISPLAY_DESC* _pDesc);
bool oDisplaySetMode(int _Index, const oDISPLAY_MODE& _Mode);

// Restores state after a call to oDisplaySetMode()
bool oDisplayResetMode(int _Index);

class oScopedDisplayMode
{	int Index;
public:
	oScopedDisplayMode(int _Index, const oDISPLAY_MODE& _Mode) : Index(_Index) { oDisplaySetMode(_Index, _Mode); }
	~oScopedDisplayMode() { oDisplayResetMode(Index); }
};

// Turns all monitors on or sets them to a low-power state
bool oDisplaySetPowerOn(bool _On = true);

// returns oInvalid if none found
int oDisplayGetPrimaryIndex();
int oDisplayGetNum();

// Returns the index of the display that is displaying the specified point in
// virtual screen space. If an oDISPLAY_DESC is specified, it is filled with the 
// same data as oDisplayEnum would return.
int oDisplayFindIndex(const int2& _ScreenPosition);

void oDisplayGetVirtualRect(int2* _pPosition, int2* _pSize);

#endif
