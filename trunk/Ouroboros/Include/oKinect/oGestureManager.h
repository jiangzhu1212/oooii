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
// The gesture manager encapsulates an oAirkeyboard and oInputMapper to produce
// a gesture system tied to oWindow. Also this exposes some hooks to be called
// from client code for hot-reload of gesture-related files using oStreamMonitor 
// and this also hooks oWindow to handle certain events. Finally this 
// encapsulation also provides straightforward rendering of the visualization of
// skeleton and other gesture camera forms. The goal is to minimize the 
// intrusion of "gesture" into application code and keep all its goings-on 
// separate and thus portable and consistent across applications.

// @tony: REFACTOR NOTE: This really ought be in oFramework, the yet-
// unrealized library that sits on top of oPlatform where code is generic but 
// uses platform API. The gesture system should not be particularly dependent
// on Kinect, but Kinect is currently the only implementation at a shippable 
// level of polish. Advise to client code: keep the ideas of oKinect and 
// oGestureManager isolated, or at least all funneled and laundered through 
// oGestureManager.

#pragma once
#ifndef oGestureManager_h
#define oGestureManager_h

#include <oPlatform/oStream.h>
#include <oGUI/window.h>

enum oGESTURE_VISUALIZATION
{
	oGESTURE_VIZ_NONE,
	oGESTURE_VIZ_COLOR,
	oGESTURE_VIZ_TRACKING,
	oGESTURE_VIZ_AIRKEYS,
	oGESTURE_VIZ_TRACKING_AND_AIRKEYS,
	oGESTURE_VISUALIZATION_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oGESTURE_VISUALIZATION)

struct oGESTURE_DEVICE_VISUALIZATION_DESC
{
	oGESTURE_DEVICE_VISUALIZATION_DESC()
		: hGestureDevice(nullptr)
		, hNotOverlay(nullptr)
		, Alignment(ouro::alignment::top_right)
		, Position(oDEFAULT, oDEFAULT)
		, ShowTimeoutMS(5000)
		, BlinkTimeoutMS(1000)
	{}

	// Representations of the gesture device to be displayed when the device 
	// becomes unplugged or unresponsive. The system internally will free the
	// resources when a manager instance is destroyed.
	ouro::icon_handle	hGestureDevice;
	ouro::icon_handle hNotOverlay;

	// Where on the screen the icon shows
	ouro::alignment::value Alignment;
	int2 Position;

	// Time before icon disappears
	int ShowTimeoutMS;

	// Time between blinks of the icon during initialization
	int BlinkTimeoutMS;
};

struct oGESTURE_VISUALIZATION_DESC
{
	oGESTURE_VISUALIZATION_DESC()
		: Visualization(oGESTURE_VIZ_TRACKING_AND_AIRKEYS)
		, Alignment(ouro::alignment::bottom_right)
		, Position(oDEFAULT, oDEFAULT)
		, Size(640, 480)
	{}

	oGESTURE_VISUALIZATION Visualization;

	// Where on the screen the icon shows
	ouro::alignment::value Alignment;
	int2 Position;
	int2 Size;
};

struct oGESTURE_MANAGER_DESC
{
	oGESTURE_MANAGER_DESC()
		: GestureCameraPitchDegrees(oDEFAULT)
		, HeadMessageTimeoutMS(2000)
		, GestureEnabled(true)
	{}

	// The angle of the gesture camera. Use oDEFAULT to "use current".
	int GestureCameraPitchDegrees;

	// Time before the icon indicating camera connectivity disappears
	// How long a combo message is displayed above someone's head.
	int HeadMessageTimeoutMS;

	// A big on/off switch for the entire system. Motivation: We deployed a early 
	// version of gesture in a department/fashion store. Then the manager there
	// organized several dummies/mannequins in front of the camera and wondered
	// why is wasn't working. So here's the quick turn-it-all-off switch.
	bool GestureEnabled;
};

struct oGESTURE_MANAGER_INIT
{
	oGESTURE_MANAGER_DESC Desc;
	oGESTURE_VISUALIZATION_DESC VizDesc;
	oGESTURE_DEVICE_VISUALIZATION_DESC DeviceVizDesc;
};

interface oGestureManager : oInterface
{
	// New settings can be set at any time. All settings take effect at the same
	// time, so often it is a good idea to Get, change the subset of parameters to 
	// change, then Set.
	virtual void GetDesc(oGESTURE_MANAGER_DESC* _pDesc) = 0;
	virtual void SetDesc(const oGESTURE_MANAGER_DESC& _Desc) = 0;
	
	// New settings can be set at any time.
	virtual void GetVisualizationDesc(oGESTURE_VISUALIZATION_DESC* _pDesc) = 0;
	virtual void SetVisualizationDesc(const oGESTURE_VISUALIZATION_DESC& _Desc) = 0;

	// New settings can be set at any time.
	virtual void GetDeviceVisualizationDesc(oGESTURE_DEVICE_VISUALIZATION_DESC* _pDesc) = 0;
	virtual void SetDeviceVisualizationDesc(const oGESTURE_DEVICE_VISUALIZATION_DESC& _Desc) = 0;

	// Currently, and somewhat awkwardly, a "gesture" is really a combo system 
	// written on top of an air keyboard. So a keyset for the air keyboard must
	// first be specified along with the combo set as currently described by the 
	// RTTI of the combo enum that will be listened for in an application's 
	// action handler as the ActionCode and an ouro::gui_action::control_activated 
	// action. More will come on what a "dynamic enum" means, but for now this API
	// specifies a gesture set.
	virtual bool SetCurrentGestureSet(const char* _KeySetName, const oRTTI* _pInputDynEnumType) = 0;

	// A "head message" is the text that appears above a user in the gesture 
	// camera debug window. Most often it is the name of a triggered event for 
	// debugging purposes.
	virtual int SetHeadMessageV(int _SkeletonIndex, const char* _Format, va_list _Args) = 0;
	inline int SetHeadMessage(int _SkeletonIndex, const char* _Format, ...) { va_list args; va_start(args, _Format); int r = SetHeadMessageV(_SkeletonIndex, _Format, args); va_end(args); return r; }
	
	// Call this between BeginOSFrame and EndOSFrame from an oGPUDevice to render 
	// gesture-related visualization according to the Desc's above visualization.
	virtual void GDIDraw(ouro::draw_context_handle _hDC, const int2& _ClientSize) = 0;

	// Call this from an oStreamMonitor callback to handle the reload of 
	// airkeyboard and combo files.
	virtual bool OnFileChange(oSTREAM_EVENT _Event, const ouro::uri_string& _ChangedURI) = 0;
};

bool oGestureManagerCreate(const oGESTURE_MANAGER_INIT& _Init, const std::shared_ptr<ouro::window>& _Window, oGestureManager** _ppGestureManager);

#endif
