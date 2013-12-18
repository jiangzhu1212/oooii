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
#include <oKinect/oKinectUtil.h>
#include <oCore/windows/win_error.h>

#ifdef oHAS_KINECT_SDK

using namespace ouro;

static_assert(NUI_SKELETON_POSITION_COUNT == ouro::skeleton_bone::count, "bone count mismatch");

DWORD oKinectGetInitFlags(oKINECT_FEATURES _Features)
{
	switch (_Features)
	{
		case oKINECT_COLOR: return NUI_INITIALIZE_FLAG_USES_COLOR;
		case oKINECT_DEPTH: return NUI_INITIALIZE_FLAG_USES_DEPTH;
		case oKINECT_COLOR_DEPTH: return NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH;
		case oKINECT_DEPTH_SKELETON_SITTING: 
		case oKINECT_DEPTH_SKELETON_STANDING: return NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX|NUI_INITIALIZE_FLAG_USES_SKELETON;
		case oKINECT_COLOR_DEPTH_SKELETON_SITTING:
		case oKINECT_COLOR_DEPTH_SKELETON_STANDING: return NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX|NUI_INITIALIZE_FLAG_USES_SKELETON;
		default: break;
	}

	return 0;
}

ouro::input_device_status::value oKinectStatusFromHR(HRESULT _hrNuiStatus)
{
	switch (_hrNuiStatus)
	{
		case S_OK: return ouro::input_device_status::ready;
		case S_NUI_INITIALIZING: return ouro::input_device_status::initializing;
		case E_NUI_NOTCONNECTED: return ouro::input_device_status::not_connected;
		case E_NUI_NOTGENUINE: return ouro::input_device_status::is_clone;
		case E_NUI_NOTSUPPORTED: return ouro::input_device_status::not_supported;
		case E_NUI_INSUFFICIENTBANDWIDTH: return ouro::input_device_status::insufficient_bandwidth;
		case E_NUI_NOTPOWERED: return ouro::input_device_status::not_powered;
		//case ???: return ouro::input_device_status::low_power;
		case E_NUI_NOTREADY: return ouro::input_device_status::not_ready;
		default: break;
	}
	return ouro::input_device_status::not_ready;
}

#define oKERR(err, msg) { std::errc::err, msg }
static const struct { std::errc::errc err; const char* msg; } sStatusErrc[] = 
{
	oKERR(already_connected, "Kinect ready"),
	oKERR(resource_unavailable_try_again, "Kinect initializing"),
	oKERR(no_such_device, "Kinect not connected"),
	oKERR(no_such_device, "Kinect-like device not supported"),
	oKERR(no_such_device, "Kinect-like device not supported"),
	oKERR(resource_unavailable_try_again, "Kinect insufficient bandwidth"),
	oKERR(protocol_error, "Kinect low power"),
	oKERR(no_such_device, "Kinect not powered"),
	oKERR(resource_unavailable_try_again, "Kinect not ready"),
};
static_assert(oCOUNTOF(sStatusErrc) == ouro::input_device_status::count, "array mismatch");

std::errc::errc oKinectGetErrcFromStatus(ouro::input_device_status::value _Status)
{
	return sStatusErrc[_Status].err;
}

const char* oKinectGetErrcStringFromStatus(ouro::input_device_status::value _Status)
{
	return sStatusErrc[_Status].msg;
}

NUI_SKELETON_POSITION_INDEX oKinectFromBone(ouro::skeleton_bone::value _Bone)
{
	return (NUI_SKELETON_POSITION_INDEX)_Bone;
}

ouro::skeleton_bone::value oKinectToBone(NUI_SKELETON_POSITION_INDEX _BoneIndex)
{
	return (ouro::skeleton_bone::value)_BoneIndex;
}

// Once done, this will need NuiImageStreamReleaseFrame called on the hStream 
// and pLatest.
void oKinectGetLatestFrame(INuiSensor* _pSensor, HANDLE _hStream, DWORD _Timeout, NUI_IMAGE_FRAME* _pLatest)
{
	HRESULT hr = S_OK;
	NUI_IMAGE_FRAME Frame;
	Frame.dwFrameNumber = oInvalid;
	Frame.liTimeStamp.QuadPart = 0;

	do
	{
		hr = _pSensor->NuiImageStreamGetNextFrame(_hStream, _Timeout, _pLatest);
		if (SUCCEEDED(hr))
		{
			if (Frame.liTimeStamp.QuadPart)
				_pSensor->NuiImageStreamReleaseFrame(_hStream, &Frame);

			if (Frame.dwFrameNumber == (_pLatest->dwFrameNumber - 1))
				break;

			Frame = *_pLatest;
		}
	}
	while (SUCCEEDED(hr));
}

ouro::surface::format oKinectGetFormat(NUI_IMAGE_TYPE _Type)
{
	// All are currently BGRA8 basically to support player index coloring.
	switch (_Type)
	{
		case NUI_IMAGE_TYPE_COLOR: 
		case NUI_IMAGE_TYPE_COLOR_YUV:
		case NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX: return ouro::surface::b8g8r8a8_unorm;
		case NUI_IMAGE_TYPE_COLOR_INFRARED: return ouro::surface::b8g8r8a8_unorm;
		case NUI_IMAGE_TYPE_COLOR_RAW_BAYER: return ouro::surface::b8g8r8a8_unorm;
		case NUI_IMAGE_TYPE_COLOR_RAW_YUV: return ouro::surface::b8g8r8a8_unorm;
		case NUI_IMAGE_TYPE_DEPTH: return ouro::surface::b8g8r8a8_unorm;
		default: break;
	}
	return ouro::surface::unknown;
}

void oKinectGetDesc(NUI_IMAGE_TYPE _Type, NUI_IMAGE_RESOLUTION _Resolution, ouro::surface::info* _pInfo)
{
	_pInfo->dimensions.z = 1;
	NuiImageResolutionToSize(_Resolution, (DWORD&)_pInfo->dimensions.x, (DWORD&)_pInfo->dimensions.y);
	_pInfo->format = oKinectGetFormat(_Type);
	_pInfo->layout = ouro::surface::image;
}

bool oKinectCreateSurface(NUI_IMAGE_TYPE _Type, NUI_IMAGE_RESOLUTION _Resolution, std::shared_ptr<ouro::surface::buffer>* _ppSurface)
{
	ouro::surface::info info;
	oKinectGetDesc(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, &info);
	*_ppSurface = ouro::surface::buffer::make(info);
	return true;
}

unsigned char oKinectGetDepthIntensity(unsigned short _Depth)
{
	// From Kinect SDK 1.7 KinectExplorer NuiImageBuffer.cpp
	// Validate arguments
	if (_Depth < oKINECT_MIN_DEPTH || _Depth > oKINECT_MAX_DEPTH)
		return UCHAR_MAX;
	// Use a logarithmic scale that shows more detail for nearer depths.
	// The constants in this formula were chosen such that values between
	// MIN_DEPTH and MAX_DEPTH will map to the full range of possible
	// byte values.
	return (BYTE)(~(BYTE)__min(UCHAR_MAX, log((double)(_Depth - oKINECT_MIN_DEPTH) / 500.0f + 1) * 74));
}

RGBQUAD oKinectGetColoredDepth(unsigned short _DepthAndIndex)
{
	static const color kPlayerColors[] =
	{ 
		Gray, Red, Orange, Yellow, Lime, Blue, 
		Indigo, Violet
	};
	static_assert(oCOUNTOF(kPlayerColors) == (1<<NUI_IMAGE_PLAYER_INDEX_SHIFT), "color count mismatch");

	const unsigned short D = NuiDepthPixelToDepth(_DepthAndIndex);
	const unsigned short I = NuiDepthPixelToPlayerIndex(_DepthAndIndex);
	const unsigned char Intensity = (D < oKINECT_MIN_DEPTH || D > oKINECT_MAX_DEPTH) ? 0 : oKinectGetDepthIntensity(D);//255 - (unsigned char)(256 * D / 0x0fff);
	const color Color = kPlayerColors[I] * (Intensity / 255.0f);
	int r,g,b;
	Color.decompose(&r, &g, &b);

	RGBQUAD q;
	q.rgbRed = (BYTE)r;
	q.rgbGreen = (BYTE)g;
	q.rgbBlue = (BYTE)b;
	q.rgbReserved = 255;
	return q;
}

void oKinectCopyBits(const NUI_IMAGE_FRAME& _NIF, ouro::surface::mapped_subresource& _Destination)
{
	NUI_LOCKED_RECT r;
	int2 Dimensions;
	NuiImageResolutionToSize(_NIF.eResolution, (DWORD&)Dimensions.x, (DWORD&)Dimensions.y);
	_NIF.pFrameTexture->LockRect(0, &r, nullptr, 0);
	switch (_NIF.eImageType)
	{
		case NUI_IMAGE_TYPE_COLOR:
			memcpy2d(_Destination.data, _Destination.row_pitch, r.pBits, r.Pitch, Dimensions.x * sizeof(RGBQUAD), Dimensions.y);
			break;

		case NUI_IMAGE_TYPE_DEPTH:
		case NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX:
			{
				const unsigned short* pSrc = (unsigned short*)r.pBits;
				RGBQUAD* pDest = (RGBQUAD*)_Destination.data;
				for (int y = 0; y < Dimensions.y; y++)
					for (int x = 0; x < Dimensions.x; x++)
						*pDest++ = oKinectGetColoredDepth(*pSrc++);
				break;
			}

		default:
			throw std::invalid_argument("unsupported NUI_IMAGE_TYPE");
	}

	_NIF.pFrameTexture->UnlockRect(0);
}

unsigned int oKinectUpdate(INuiSensor* _pSensor, HANDLE _hStream, ouro::surface::buffer* _pSurface)
{
	unsigned int FrameNumber = oInvalid;

	NUI_IMAGE_FRAME NIF;
	oKinectGetLatestFrame(_pSensor, _hStream, 10000, &NIF);

	FrameNumber = NIF.dwFrameNumber;
	ouro::surface::mapped_subresource Mapped;
	int2 ByteDimensions;
	_pSurface->map(0, &Mapped, &ByteDimensions);
	oKinectCopyBits(NIF, Mapped);
	_pSurface->unmap(0);

	oV(_pSensor->NuiImageStreamReleaseFrame(_hStream, &NIF));
	return FrameNumber;
}

int2 oKinectSkeletonToScreen(
	const float4& _CameraSpacePosition
	, const int2& _TargetPosition
	, const int2& _TargetDimensions
	, const int2& _DepthBufferResolution)
{
	int2 Screen(oDEFAULT, oDEFAULT);

	if (_CameraSpacePosition.w >= 0.0f)
	{
		USHORT depth;
		NuiTransformSkeletonToDepthImage((const Vector4&)_CameraSpacePosition, (LONG*)&Screen.x, (LONG*)&Screen.y, &depth);
		Screen = Screen * _TargetDimensions / /*_DepthBufferResolution*/int2(320,240); // this seems hard-coded
		Screen += _TargetPosition;
	}

	return Screen;
}

int oKinectCalcScreenSpacePositions(
	const oGUI_BONE_DESC& _Skeleton
	, const int2& _TargetPosition
	, const int2& _TargetDimensions
	, const int2& _DepthBufferResolution
	, int2 _ScreenSpacePositions[ouro::skeleton_bone::count])
{
	int NumValid = 0;
	for (size_t i = 0; i < ouro::skeleton_bone::count; i++)
	{
		_ScreenSpacePositions[i] = oKinectSkeletonToScreen(_Skeleton.Positions[i], _TargetPosition, _TargetDimensions, _DepthBufferResolution);
		if (_ScreenSpacePositions[i].x != oDEFAULT)
			NumValid++;
	}

	return NumValid;
}

#endif // oHAS_KINECT_SDK

void oKinectCalcBoneSpacePositions(ouro::skeleton_bone::value _OriginBone, oGUI_BONE_DESC& _Skeleton)
{
	const float3 Offset = _Skeleton.Positions[_OriginBone].xyz();
	oFOR(auto& P, _Skeleton.Positions)
	{
		float3 NewPos = P.xyz() - Offset;
		P.x = NewPos.x;
		P.y = NewPos.y;
		P.z = NewPos.z;
	}
}
