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
#ifndef oKinect_h
#define oKinect_h

#include <oPlatform/oCamera.h>

#define MAX_NUM_OF_SENSORS 6

interface oKinect : oInterface
{
	struct DESC
	{
		bool UseStartupTilt;
		float StartupTilt;
		float CutoffDistance;
		float3 CameraWorldPosition;
		float3 AdditionalYawPitchRoll;
		int MaxSimultaneousUsers;
		bool UseNearMode;
	};

	typedef oFUNCTION<void(const oGUI_ACTION_DESC& _Action)> ActionUpdateCallback;

	virtual interface INuiSensor* GetSensor(int sensorId) = 0;
	virtual void SetUseNearMode(bool val) = 0;
	virtual void UpdateCameraAngle(int _SensorId) = 0;
	virtual void RegisterActionCallback(ActionUpdateCallback _Callback) = 0;
	virtual void ReleaseActionCallback() = 0;
	virtual void RegisterDisplayObserver(const oCameraOnFrameFn& _OnFrame) = 0;
	virtual void RemoveDisplayObservers() = 0;
	virtual bool SetPitch(float _AngleInDegrees) = 0;
	virtual bool GetPitch(float* _pAngleInDegrees) const = 0;
	virtual bool GetEstimatedCameraPosition(float3* _pEstimatedCameraPosition) const = 0;
	virtual bool GetCameraOffset(float3 *_pCameraOffset) const = 0;
};

bool oKinectCreate(const oKinect::DESC& _Desc, const oKinect::ActionUpdateCallback& _Callback, oKinect** _ppKinect);

bool oKinectFrameStreamCreate(oKinect* _pKinect, oCameraFrameStream** _ppFrameStream);
bool oKinectArticulatorCreate(oKinect* _pKinect, oCameraArticulator** _ppArticulator);
bool oKinectPositionCreate(oKinect* _pKinect, oCameraPosition** _ppPosition);

#endif