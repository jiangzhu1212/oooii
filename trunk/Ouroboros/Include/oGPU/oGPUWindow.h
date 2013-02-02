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
// A specialization of oWindow who's client area is managed by an oGPU_ device
// interface. Mainly, this is the operating system representation of where final
// rendering and presentation to the user is done. Sepcifically, this is not
// meant to be a GUI-type window, it is ONLY the control to which rendering 
// occurs. If there is to be UI/menus around rendering, embed this window as 
// borderless in a larger window.

#pragma once
#ifndef oGPUWindow_h
#define oGPUWindow_h

#include <oPlatform/oWindow.h>
#include <oGPU/oGPU.h>

interface oGPUWindow : oWindow
{
	virtual void GetDevice(oGPUDevice** _ppDevice) const threadsafe = 0;

	// Renders the next frame and pauses. Calling Step(false) will restore normal
	// rendering.
	virtual void Step(bool _UseStepping = true) threadsafe = 0;

	// Return the number of times the Render function has been called and 
	// presented as the front buffer.
	virtual int GetFrameCount() const threadsafe = 0;
};

struct oGPU_WINDOW_INIT : oWINDOW_INIT
{
	// NOTE: Calling the RenderFunction and flipping the backbuffer (presenting) 
	// is done during the oWindow's oGUI_IDLE event. If EnableIdleEvent is false
	// in the WinDesc portion of init (or at any time using Map/Unmap) then 
	// rendering will cease/pause.

	oGPU_WINDOW_INIT()
		: VSynced(true)
		, UIRenderingCompatible(true)
		, StartStepping(false)
		, DepthStencilFormat(oSURFACE_UNKNOWN)
	{
		WindowThreadDebugName = "oGPUWindow Message Thread";
	}

	// Primary render target should not be retained by the system because it can
	// be created and destroyed depending on oWindow resize/fullscreen/etc. It is 
	// safe to use for the duration of the RenderFunction call, but is not valid
	// outside it.
	oFUNCTION<void(oGPURenderTarget* _pPrimaryRenderTarget)> RenderFunction;
	oSURFACE_FORMAT DepthStencilFormat; // specify UNKNOWN for no DS buffer
	bool VSynced;

	// If true, the main swap chain is flagged to interoperate with the native
	// operating system's drawing system (i.e. GDI on Windows). This can crash 
	// tools such as NVIDIA's Parallel Insight and is often not as well a 
	// supported path as others, but allows for some quick-and-dirty HUD/stats
	// during bring-up.
	bool UIRenderingCompatible;

	// Start in stepping mode paused on frame 0. Use Step() to increment one frame
	// at a time, or Step(false) to render at full rate.
	bool StartStepping;
};

bool oGPUWindowCreate(const oGPU_WINDOW_INIT& _Init, oGPUDevice* _pDevice, threadsafe oGPUWindow** _ppGPUWindow);

#endif
