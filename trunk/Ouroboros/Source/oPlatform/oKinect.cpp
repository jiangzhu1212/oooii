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
#include <oBasis/oX11KeyboardSymbols.h>
#include <oPlatform/oKinect.h>
#include "SoftLink/oWinKinect10.h"
// Temp: hook up the display window directly to Kinect
#include "oKinectDisplay.h"

// Windows is a bit weird in that all KB/Mouse AND TOUCH are handled through a
// single message pump, so really an HWND is an event handler that happens to 
// have a visual component... but not for kinect. Also it doesn't seem in 
// keeping with the Kinect SDK license to publish header/libs, so we need to 
// create builds that do not link to Kinect SDK.
#ifdef oHAS_KINECT_SDK

const double UserTrackingTimeoutInSeconds = 2;

const oGUID& oGetGUID(threadsafe const oKinect* threadsafe const*)
{
	// {15DCA2B2-72E2-4fd7-A6A4-FCAE7B184D91}
	static const oGUID oIIDKinect = { 0x15dca2b2, 0x72e2, 0x4fd7, { 0xa6, 0xa4, 0xfc, 0xae, 0x7b, 0x18, 0x4d, 0x91 } };
	return oIIDKinect;
}

struct oKinectFrameStream : public oCameraFrameStream
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oCameraFrameStream);

	oKinectFrameStream(oKinect* _pKinect, bool* _pSuccess) : Kinect(_pKinect) { *_pSuccess = !!_pKinect; }
	bool AddObserver(const oCameraOnFrameFn& _OnFrame) override;
	bool RemoveObservers() override;
private:
	oRef<oKinect> Kinect;
	oRefCount RefCount;
};

bool oKinectFrameStream::AddObserver(const oCameraOnFrameFn& _OnFrame)
{ 
	if (Kinect)
	{
		Kinect->RegisterDisplayObserver(_OnFrame);
		return true;
	}
	return false; 
}

bool oKinectFrameStream::RemoveObservers()
{ 
	if (Kinect)
	{
		Kinect->RemoveDisplayObservers();
		return true;
	}
	return false; 
}

struct oKinectArticulator : public oCameraArticulator
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oCameraArticulator);

	oKinectArticulator(oKinect* _pKinect, bool* _pSuccess) : Kinect(_pKinect) { *_pSuccess = !!_pKinect; };

	bool SetPitch(float _AngleInDegrees) override { return Kinect ? Kinect->SetPitch(_AngleInDegrees) : false; }
	bool GetPitch(float* _pAngleInDegrees) override { return Kinect ? Kinect->GetPitch(_pAngleInDegrees) : false; };
private:
	oRef<oKinect> Kinect;
	oRefCount RefCount;
};

struct oKinectPosition : public oCameraPosition
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oCameraArticulator);

	oKinectPosition(oKinect* _pKinect, bool* _pSuccess) : Kinect(_pKinect) { *_pSuccess = !!_pKinect; };

	bool GetPosition(float3* _pCameraPositionOffset) override { return Kinect ? Kinect->GetCameraOffset(_pCameraPositionOffset) : false; }
	bool GetEstimatedPosition(float3* _pEstimatedPosition) override { return Kinect ? Kinect->GetEstimatedCameraPosition(_pEstimatedPosition) : false; }

private:
	oRef<oKinect> Kinect;
	oRefCount RefCount;
};

struct oKinect_Impl : public oKinect
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oKinect);

	oKinect_Impl(const DESC& _Desc, ActionUpdateCallback _Callback, bool* _pSuccess);
	~oKinect_Impl();

	void Update();
	INuiSensor* GetSensor(int sensorId) override {return NuiSensors[sensorId];}
	void SetUseNearMode(bool val) override;
	void UpdateCameraAngle(int _SensorId) override;
	void RegisterActionCallback(ActionUpdateCallback _Callback) override { GuiActionCallback = _Callback; }
	void ReleaseActionCallback() override { GuiActionCallback = nullptr; }
	void RegisterDisplayObserver(const oCameraOnFrameFn& _OnFrame) override { Display.RegisterObserver(_OnFrame); }
	void RemoveDisplayObservers() override { Display.RemoveObservers(); }
	bool SetPitch(float _AngleInDegrees) override;
	bool GetPitch(float* _pAngleInDegrees) const override;

	// Kinect can currently only estimate its height
	bool GetEstimatedCameraPosition(float3* _pEstimatedCameraPosition) const override { *_pEstimatedCameraPosition = float3(0, EstimatedCameraHeightInMeters, 0); return true; };
	bool GetCameraOffset(float3 *_pCameraOffset) const override { *_pCameraOffset = KinectDesc.CameraWorldPosition; return true; }

private:
	void UpdateUserPositions(NUI_SKELETON_FRAME SkeletonFrame, int sensorIdx);
	void Init();
	void Shutdown();
	void Nui_GotDepthAlert();
	void Nui_GotSkeletonAlert();
	static void CALLBACK KinectConnectionEventS(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void* pUserData);
	void CALLBACK KinectConnectionEvent( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName );
	void UpdateTrackingAction(int _UserIdx, int _BoneIdx, const float3 &_Position);

	oRef<threadsafe oDispatchQueuePrivate> DispatchQueue;
	// Temp: display window
	oKinectDisplay Display;

	INuiSensor*	NuiSensors[MAX_NUM_OF_SENSORS];
	DWORD CurrentFlags;
	bool UseNearMode;
	int	NumOfSensors;
	oRefCount RefCount;
	oKinect::DESC KinectDesc;
	long CameraAngle;
	float EstimatedCameraHeightInMeters;
	// Handle used for Kinect SDK's depth stream
	HANDLE pDepthStreamHandle;
	HANDLE hEvNuiProcessStop;
	HANDLE hNextDepthFrameEvent;
	HANDLE hNextSkeletonEvent;

	// Animated bar displayed while Kinect is initializing
	//oWindowUITest ProgressBar;
	//oGestureEditUI GestureEditUI;

	struct TrackedUserTimer
	{
		bool bTracked;
		double Time;
	};
	TrackedUserTimer TrackedUsers[6];
	oStringS LastZoneName;
	ActionUpdateCallback GuiActionCallback;
};

void CALLBACK oKinect_Impl::KinectConnectionEventS( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void*  pUserData )
{
    reinterpret_cast<oKinect_Impl* >(pUserData)->KinectConnectionEvent( hrStatus, instanceName, uniqueDeviceName );
}

void CALLBACK oKinect_Impl::KinectConnectionEvent( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName )
{
    if (SUCCEEDED(hrStatus))      
    {
			//ProgressBar.SetTitle("A Kinect has been found.  Initializing...");
			if (S_OK == hrStatus)
			{
				// Initialize the Kinect sensor identified by the instanceName parameter.
				if (!NuiSensors[0])
				{
					HRESULT hr = oWinKinect10::Singleton()->NuiCreateSensorById( instanceName, &NuiSensors[0] );
   
					// Generic creation failure
					if (FAILED(hr))
					{
						//ProgressBar.SetTitle("Kinect initialization failed!!");
						return;
					}

					Init();
				}
			}
    }
    else      
    {          
			// Kinect was disconnected   
			NumOfSensors=0;
			if (NuiSensors[0])
			{
				NuiSensors[0]->NuiShutdown();
				NuiSensors[0]->Release();
				NuiSensors[0] = nullptr;
			}

			//#ifdef _DEBUG
			//	ProgressBar.Init();
			//	ProgressBar.SetTitle("Kinect has been disconnected...");
			//#endif
    }
}

oKinect_Impl::oKinect_Impl(const DESC& _Desc, ActionUpdateCallback _Callback, bool* _pSuccess)
	:NumOfSensors(0)
	,KinectDesc(_Desc)
	,GuiActionCallback(_Callback)
	,EstimatedCameraHeightInMeters(0.0f)
{
	*_pSuccess = false;

	for (int i = 0; i < 6; ++i)
	{
		TrackedUsers[i].bTracked = false;
	}

	hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
	hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	oKinect* pThis = this;

	// If calling NuiInitialize and NuiShutdown multiple times within the life time of hte program this MUST be called
	// prior to calling NuiInitialize (NuiInitialize is called in Init below)
	// http://www.microsoft.com/en-us/kinectforwindows/develop/release-notes.aspx#_6._known_issues
	oWinKinect10::Singleton()->NuiSetDeviceStatusCallback( &oKinect_Impl::KinectConnectionEventS, pThis );

//#ifdef _DEBUG
//	ProgressBar.Init();
//	ProgressBar.SetTitle("Looking for Kinect(s)...");
//#endif

	for (int i=0; i<MAX_NUM_OF_SENSORS; i++)
		NuiSensors[i] = nullptr;

	HRESULT hr = oWinKinect10::Singleton()->NuiCreateSensorByIndex(0, &NuiSensors[0]); 
	if (FAILED(hr))
	{
		*_pSuccess = false;
		oErrorSetLast(oERROR_NOT_FOUND, "Failed to Load Kinect Sensor. Make sure a Kinect is installed.");
		return;
	}
	Init();

	//if (KinectDesc.ShowUI)
	//	GestureEditUI.Init(this);

	if (!oDispatchQueueCreatePrivate("oKinect thread", 2, &DispatchQueue))
		return;

	DispatchQueue->Dispatch(&oKinect_Impl::Update, this);

	*_pSuccess = true;
}

oKinect_Impl::~oKinect_Impl()
{
	// Signal the Nui processing thread to stop
	SetEvent(hEvNuiProcessStop);

	if (DispatchQueue)
		DispatchQueue->Join();
	CloseHandle(hEvNuiProcessStop);

 	Shutdown();
}

bool oKinect_Impl::SetPitch(float _AngleInDegrees)
{
	bool success = false;

	INuiSensor* tmpSensor = GetSensor(0);
	if (tmpSensor)
	{
		HRESULT hr = tmpSensor->NuiCameraElevationSetAngle((LONG)_AngleInDegrees);
		if (!FAILED(hr))
			success = true;
	}

	return success;
}

bool oKinect_Impl::GetPitch(float* _pAngleInDegrees) const
{
	*_pAngleInDegrees = (float)CameraAngle;
	return true;
}

void oKinect_Impl::SetUseNearMode(bool val)
{
	DWORD flags = (val) ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE | NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES : 0;

	for (int sensorIdx=0; sensorIdx < MAX_NUM_OF_SENSORS; sensorIdx++)
	{
		INuiSensor* tempSensor = GetSensor(sensorIdx);
		if (tempSensor)
			tempSensor->NuiImageStreamSetImageFrameFlags(pDepthStreamHandle, flags);
	}

	UseNearMode = val;
}

void oKinect_Impl::UpdateCameraAngle(int _SensorId)
{
	INuiSensor* tmpSensor = GetSensor(_SensorId);
	if (tmpSensor)
		tmpSensor->NuiCameraElevationGetAngle(&CameraAngle);
}

// Find and initialize all connected sensors
void oKinect_Impl::Init()
{
	bool bSkeletonTrackingEnabled = false;
	//for(int sensorIdx=0; sensorIdx < MAX_NUM_OF_SENSORS; sensorIdx++)
	{
		INuiSensor* tmpSensor = GetSensor(0);
		if (tmpSensor)
		{
			//ProgressBar.SetTitle("oKinect is Loading...");

			// Initialize Kinect sensor with color camera, depth sensor, & skeleton tracking.
			CurrentFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
			HRESULT hr = tmpSensor->NuiInitialize( CurrentFlags );

			// Current version 1.0.3.190, supports multiple sensors, but only supports one sensor using skelton tracking at any time.
			if (E_NUI_SKELETAL_ENGINE_BUSY == hr)
			{
				CurrentFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
				hr = tmpSensor->NuiInitialize( CurrentFlags );
			}

			// If this sensor was initialized with skeleton tracking, enable it.
			if ( HasSkeletalEngine( tmpSensor ))
			{
				hr = tmpSensor->NuiSkeletonTrackingEnable(hNextSkeletonEvent, NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS);
				if (!FAILED(hr))
				{
					bSkeletonTrackingEnabled = true;

					// Set camera to lowest angle.  Use + - keys to manually change elevation.
					if (KinectDesc.UseStartupTilt)
						tmpSensor->NuiCameraElevationSetAngle((LONG)KinectDesc.StartupTilt);
				}
			}

			hr = tmpSensor->NuiImageStreamOpen(
				HasSkeletalEngine(tmpSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
				oDEPTH_RESOLUTION,
				0,
				2,
				hNextDepthFrameEvent,
				&pDepthStreamHandle );

			//NuiSensors[sensorIdx] = tmpSensor;
			NuiSensors[0] = tmpSensor;
			NumOfSensors = 1;
		}
	}
	UpdateCameraAngle(0);

	//if (NumOfSensors == 0)
	//{
	//	ProgressBar.SetTitle("Waiting for Kinect to be plugged in...");
	//}

	// Show the user position only if we're not showing the skeleton.
	UseNearMode = (!bSkeletonTrackingEnabled || KinectDesc.UseNearMode) ? true : false;
	SetUseNearMode(UseNearMode);
}

// Release all sensors.
void oKinect_Impl::Shutdown()
{
	//if (KinectDesc.ShowUI)
	//	GestureEditUI.Close();
	oSleep(1000);  // Leave message up long enough for user to read before closing.

	for (int sensorIdx=0; sensorIdx < MAX_NUM_OF_SENSORS; sensorIdx++)
	{
		if(NuiSensors[sensorIdx])
		{
			NuiSensors[sensorIdx]->NuiShutdown();
			NuiSensors[sensorIdx]->Release();
			NuiSensors[sensorIdx] = nullptr;
		}
	}

    if (hNextSkeletonEvent && (hNextSkeletonEvent != INVALID_HANDLE_VALUE))
    {
        CloseHandle(hNextSkeletonEvent);
        hNextSkeletonEvent = NULL;
    }
    if (hNextDepthFrameEvent && (hNextDepthFrameEvent != INVALID_HANDLE_VALUE))
    {
        CloseHandle(hNextDepthFrameEvent);
        hNextDepthFrameEvent = NULL;
    }

	pDepthStreamHandle = nullptr;
	NumOfSensors = 0;
}

// Get the user's world position, assuming the kinect is NOT rotated along the z-axis (roll)
float3 GetAdjustedUserPosition(float4 _position, float _yaw, float _pitch, float3 _cameraWorldPosition)
{
	float reversePitch = radians(-_pitch);
	float rotatedY = _position.y*  cos(reversePitch) - _position.z*  sin(reversePitch);
	float rotatedZ = _position.y*  sin(reversePitch) + _position.z*  cos(reversePitch);
	float3 retVal(_position.x, _cameraWorldPosition.y + rotatedY, _cameraWorldPosition.z + rotatedZ);

	float reverseYaw = radians(-_yaw);
	float rotatedX = _position.x*  cos(reverseYaw) - _position.z*  sin(reverseYaw);
	rotatedZ = _position.x*  sin(reverseYaw) + _position.z*  cos(reverseYaw);
	retVal.x = _cameraWorldPosition.x + rotatedX;
	retVal.z = rotatedZ;

	return retVal;
}

void oKinect_Impl::UpdateTrackingAction(int _UserIdx, int _BoneIdx, const float3 &_Position)
{
	oKEYBOARD_KEY key = oKB_None;

	bool validBone = true;
	switch (_BoneIdx)
	{
	case NUI_SKELETON_POSITION_HEAD:
		key = oKB_Pos_Head;
		break;
	case NUI_SKELETON_POSITION_HIP_CENTER:
		key = oKB_Pos_Hip;
		break;
	case NUI_SKELETON_POSITION_HAND_LEFT:
		key = oKB_Pos_LeftHand;
		break;
	case NUI_SKELETON_POSITION_HAND_RIGHT:
		key = oKB_Pos_RightHand;
		break;
	default:
		validBone = false;
		break;
	}

	if (validBone)
	{
		if (!TrackedUsers[_UserIdx].bTracked)
		{
			// Send acquired event
			oGUI_ACTION_DESC acquiredActionDesc;
			acquiredActionDesc.SourceID = _UserIdx;
			acquiredActionDesc.Key = oKB_None;
			acquiredActionDesc.Action = oGUI_ACTION_CONTROL_ACTIVATED;
			TrackedUsers[_UserIdx].bTracked = true;
			if (GuiActionCallback)
				GuiActionCallback(acquiredActionDesc);
		}

		// Update tracked time
		TrackedUsers[_UserIdx].Time = oTimer();

		oGUI_ACTION_DESC actionDesc;
		actionDesc.SourceID = _UserIdx;
		actionDesc.Key = key;
		actionDesc.Action = oGUI_ACTION_POINTER_MOVE;
		actionDesc.PointerPosition = _Position;
		if (GuiActionCallback)
			GuiActionCallback(actionDesc);
	}
}

void oKinect_Impl::UpdateUserPositions(NUI_SKELETON_FRAME SkeletonFrame, int _SensorId)
{
	for (int userIdx = 0 ; userIdx < NUI_SKELETON_COUNT ; userIdx++)
	{
		if (SkeletonFrame.SkeletonData[userIdx].eTrackingState == NUI_SKELETON_TRACKED || SkeletonFrame.SkeletonData[userIdx].eTrackingState == NUI_SKELETON_POSITION_ONLY)
		{
			EstimatedCameraHeightInMeters = SkeletonFrame.vFloorClipPlane.w;

			// Near mode only effects user hip bone position.
			if (UseNearMode)
			{
				// near mode only draw bones once
				Display.DrawBones(SkeletonFrame.SkeletonData[userIdx].Position, 10);
				Vector4 posToUse = SkeletonFrame.SkeletonData[userIdx].Position;
				// Get world position coordinates
				float cameraHeight = EstimatedCameraHeightInMeters > 0 ? EstimatedCameraHeightInMeters : KinectDesc.CameraWorldPosition.z;
				float3 adjustedPos = GetAdjustedUserPosition(
					float4(posToUse.x, posToUse.y, posToUse.z, posToUse.w), 
					KinectDesc.AdditionalYawPitchRoll.x, 
					(float)CameraAngle, 
					float3(KinectDesc.CameraWorldPosition.x, cameraHeight, KinectDesc.CameraWorldPosition.z) );

				// send event oKB_Pos_Hip with SouceID userIdx

				UpdateTrackingAction(userIdx, NUI_SKELETON_POSITION_HIP_CENTER, adjustedPos);
			}
			else if (SkeletonFrame.SkeletonData[userIdx].eTrackingState == NUI_SKELETON_TRACKED)
			{
				// if in near mode, all bones get assigned the same location, but we usually only sample the hip
				for(int boneIdx=0; boneIdx<NUI_SKELETON_POSITION_COUNT; boneIdx++)
				{
					Vector4 posToUse = (UseNearMode) ?	SkeletonFrame.SkeletonData[userIdx].Position : SkeletonFrame.SkeletonData[userIdx].SkeletonPositions[boneIdx];
					// Get world position coordinates
					float cameraHeight = EstimatedCameraHeightInMeters > 0 ? EstimatedCameraHeightInMeters : KinectDesc.CameraWorldPosition.z;
					float3 adjustedPos = GetAdjustedUserPosition( 
						float4(posToUse.x, posToUse.y, posToUse.z, posToUse.w), 
						KinectDesc.AdditionalYawPitchRoll.x, 
						(float)CameraAngle, 
						float3(KinectDesc.CameraWorldPosition.x, cameraHeight, KinectDesc.CameraWorldPosition.z) );

					// Translate kinect bone to oTrackingSkeleton.Bone
					//trackingSkeleton.Bones[boneIdx].Position = adjustedPos; // oTrackingSkeleton bones and kinect bones are in the same order
					
					// Update display
					Display.DrawBones(SkeletonFrame.SkeletonData[userIdx].SkeletonPositions[boneIdx], 3);

					// Send an action event
					UpdateTrackingAction(userIdx, boneIdx, adjustedPos);
				}
			}
			else
			{
				// not tracked this frame
				if (TrackedUsers[userIdx].bTracked)
				{
					// Send lost event
					oGUI_ACTION_DESC lostActionDesc;
					lostActionDesc.SourceID = userIdx;
					lostActionDesc.Key = oKB_None;
					lostActionDesc.Action = oGUI_ACTION_CONTROL_DEACTIVATED;
					TrackedUsers[userIdx].bTracked = false;
					if (GuiActionCallback)
						GuiActionCallback(lostActionDesc);
				}
			}

		}
	}
}

// Checks for updates to Kinect's depth stream and joint tracking.
void oKinect_Impl::Update()
{
	const int numEvents = 3;
	HANDLE hEvents[numEvents] = { hEvNuiProcessStop, hNextDepthFrameEvent, hNextSkeletonEvent };
	int    nEventIdx;

	// Wait for any of the events to be signalled
	nEventIdx = WaitForMultipleObjects(numEvents, hEvents, FALSE, 100);

	// Process signal events
	switch (nEventIdx)
	{
	case WAIT_TIMEOUT:
		break;

	case WAIT_OBJECT_0 + 1:
		Nui_GotDepthAlert();
		break;

	case WAIT_OBJECT_0 + 2:
		Nui_GotSkeletonAlert();
		break;
	}

	// look for time outs
	for (int i = 0; i < 6; ++i)
	{
		// not tracked this frame
		if (TrackedUsers[i].bTracked && oTimer() - TrackedUsers[i].Time > UserTrackingTimeoutInSeconds)
		{
			// Send lost event
			oGUI_ACTION_DESC lostActionDesc;
			lostActionDesc.SourceID = i;
			lostActionDesc.Key = oKB_None;
			lostActionDesc.Action = oGUI_ACTION_CONTROL_DEACTIVATED;
			TrackedUsers[i].bTracked = false;
			if (GuiActionCallback)
				GuiActionCallback(lostActionDesc);
		}
	}

	DispatchQueue->Dispatch(&oKinect_Impl::Update, this);
}

void oKinect_Impl::Nui_GotDepthAlert()
{
	// Close the progress bar
	//if (ProgressBar.IsOpen())
	//	ProgressBar.Close();

	for (int sensorIdx=0; sensorIdx < MAX_NUM_OF_SENSORS; sensorIdx++)
	{
		INuiSensor* tmpSensor = GetSensor(sensorIdx);
		if (tmpSensor)
		{
			NUI_IMAGE_FRAME imageFrame;

			// This call appears to be necessary for skeletal tracking
			HRESULT hr = tmpSensor->NuiImageStreamGetNextFrame(
				pDepthStreamHandle,
				0,
				&imageFrame);

			if (FAILED( hr ))
			{
				return;
			}

			// Temp
			Display.CreateDepthDisplayImage(imageFrame);
			tmpSensor->NuiImageStreamReleaseFrame(pDepthStreamHandle, &imageFrame);
		}
	}

}

void oKinect_Impl::Nui_GotSkeletonAlert()
{
	bool bForceRetrack = false;
	DWORD dwTrackingIDs[NUI_SKELETON_MAX_TRACKED_COUNT] = {0,0};
	const int MAX_DIST = 99999;
	float closestUserDist = MAX_DIST;
	float nextClosestUserDist = MAX_DIST;

	for (int sensorIdx=0; sensorIdx < MAX_NUM_OF_SENSORS; sensorIdx++)
	{
		INuiSensor* tmpSensor = GetSensor(sensorIdx);
		if (tmpSensor)
		{
			NUI_SKELETON_FRAME SkeletonFrame = {0};

			bool bFoundSkeleton = false;

			if (SUCCEEDED(tmpSensor->NuiSkeletonGetNextFrame(0, &SkeletonFrame)))
			{
				for (int i = 0 ; i < NUI_SKELETON_COUNT ; i++)
				{
					if (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ||
						(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY))
					{
						float tempDist = distance(float3(SkeletonFrame.SkeletonData[i].Position.x, SkeletonFrame.SkeletonData[i].Position.y, SkeletonFrame.SkeletonData[i].Position.z), float3(0,0,0));
						if(tempDist > KinectDesc.CutoffDistance)
							continue;

						bFoundSkeleton = true;

						if (closestUserDist == MAX_DIST || tempDist < closestUserDist)
						{
							nextClosestUserDist = MAX_DIST;
							dwTrackingIDs[1] = dwTrackingIDs[0];
							dwTrackingIDs[0] = SkeletonFrame.SkeletonData[i].dwTrackingID;
							if(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY)
								bForceRetrack = true;
						}
						else if (nextClosestUserDist == MAX_DIST || tempDist < nextClosestUserDist)
						{
							dwTrackingIDs[1] = SkeletonFrame.SkeletonData[i].dwTrackingID;
							if(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY)
								bForceRetrack = true;
						}
					}
				}
			}

			if (KinectDesc.MaxSimultaneousUsers < 2)
				dwTrackingIDs[1] = 0;

			if (bForceRetrack)
				tmpSensor->NuiSkeletonSetTrackedSkeletons(dwTrackingIDs);

			UpdateUserPositions(SkeletonFrame, sensorIdx);
		}
	}
}

#endif

bool oKinectCreate(const oKinect::DESC& _Desc, const oKinect::ActionUpdateCallback& _Callback, oKinect** _ppKinect)
{
	#ifdef oHAS_KINECT_SDK
		bool success = false;
		oCONSTRUCT(_ppKinect, oKinect_Impl(_Desc, _Callback, &success));
		return !!*_ppKinect;
	#else
		return oErrorSetLast(oERROR_PLATFORM, "oPlatform was not compiled with oHAS_KINECT defined. This must be defined to enable Kinect since the SDK is not allowed to include Kinect SDK headers/libs in the distribution. See SDK/Ouroboros/External/Kinect/Readme.txt for more details.");
	#endif
}

bool oKinectFrameStreamCreate(oKinect* _pKinect, oCameraFrameStream** _ppFrameStream)
{
	bool success = false;
	#ifdef oHAS_KINECT_SDK
		oCONSTRUCT(_ppFrameStream, oKinectFrameStream(_pKinect, &success));
	#endif
	return success;
}

bool oKinectPositionCreate(oKinect* _pKinect, oCameraPosition** _ppPosition)
{
	bool success = false;
	#ifdef oHAS_KINECT_SDK
		oCONSTRUCT(_ppPosition, oKinectPosition(_pKinect, &success));
	#endif
	return success;
}

bool oKinectArticulatorCreate(oKinect* _pKinect, oCameraArticulator** _ppArticulator)
{
	bool success = false;
	#ifdef oHAS_KINECT_SDK
		oCONSTRUCT(_ppArticulator, oKinectArticulator(_pKinect, &success));
	#endif
	return success;
}
