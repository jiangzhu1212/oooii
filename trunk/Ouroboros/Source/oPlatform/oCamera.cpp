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
#include <oPlatform/oCamera.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oLockThis.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oMemory.h>
#include <oConcurrency/mutex.h>
#include <oStd/fixed_string.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWindows.h>
#include <dshow.h>
#include <assert.h>
#include <vector>
#include "oCamera_QEdit.h"

// 73646976-0000-0010-8000-00AA00389B71
static const oGUID oGUID_MEDIATYPE_Video = { 0x73646976, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
// e436eb7b-524f-11ce-9f53-0020af0ba770
static const oGUID oGUID_MEDIASUBTYPE_RGB565 = { 0xe436eb7b, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
// e436eb7d-524f-11ce-9f53-0020af0ba770
static const oGUID oGUID_MEDIASUBTYPE_RGB24 = { 0xe436eb7d, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
// 773c9ac0-3274-11d0-B724-00aa006c1A01
static const oGUID oGUID_MEDIASUBTYPE_ARGB32 = { 0x773c9ac0, 0x3274, 0x11d0, { 0xb7, 0x24, 0x0, 0xaa, 0x0, 0x6c, 0x1a, 0x1 } };
// e436eb7e-524f-11ce-9f53-0020af0ba770
static const oGUID oGUID_MEDIASUBTYPE_RGB32 = { 0xe436eb7e, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
// e436ebb3-524f-11ce-9f53-0020af0ba770
static const oGUID oGUID_CLSID_FilterGraph = { 0xe436ebb3, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
// BF87B6E1-8C27-11d0-B3F0-00AA003761C5
static const oGUID oGUID_CLSID_CaptureGraphBuilder2 = { 0xBF87B6E1, 0x8C27, 0x11d0, { 0xB3, 0xF0, 0x0, 0xAA, 0x00, 0x37, 0x61, 0xC5 } };
// 860BB310-5D01-11d0-BD3B-00A0C911CE86
static const oGUID oGUID_CLSID_VideoInputDeviceCategory = { 0x860BB310, 0x5D01, 0x11d0, { 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };
// 62BE5D10-60EB-11d0-BD3B-00A0C911CE86
static const oGUID oGUID_CLSID_SystemDeviceEnum = { 0x62BE5D10, 0x60EB, 0x11d0, { 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };
// C1F400A0-3F08-11d3-9F0B-006008039E37
static const oGUID oGUID_CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
// 29840822-5B84-11D0-BD3B-00A0C911CE86
static const oGUID oGUID_IID_ICreateDevEnum = { 0x29840822, 0x5B84, 0x11D0, { 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };
// 56a86895-0ad4-11ce-b03a-0020af0ba770
static const oGUID oGUID_IID_IBaseFilter = { 0x56a86895, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
// C6E13340-30AC-11d0-A18C-00A0C9118956
static const oGUID oGUID_IID_IAMStreamConfig = { 0xC6E13340, 0x30AC, 0x11d0, { 0xA1, 0x8C, 0x00, 0xA0, 0xC9, 0x11, 0x89, 0x56 } };

// The following code is from http://msdn.microsoft.com/en-us/library/dd407288(v=VS.85).aspx
namespace SampleCode {

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

// Query whether a pin has a specified direction (input / output)
HRESULT IsPinDirection(IPin *pPin, PIN_DIRECTION dir, BOOL *pResult)
{
	PIN_DIRECTION pinDir;
	HRESULT hr = pPin->QueryDirection(&pinDir);
	if (SUCCEEDED(hr))
	{
		*pResult = (pinDir == dir);
	}
	return hr;
}

// Query whether a pin is connected to another pin.
//
// Note: This function does not return a pointer to the connected pin.
static HRESULT IsPinConnected(IPin *pPin, BOOL *pResult)
{
	IPin *pTmp = nullptr;
	HRESULT hr = pPin->ConnectedTo(&pTmp);
	if (SUCCEEDED(hr))
	{
		*pResult = TRUE;
	}
	else if (hr == VFW_E_NOT_CONNECTED)
	{
		// The pin is not connected. This is not an error for our purposes.
		*pResult = FALSE;
		hr = S_OK;
	}

	SafeRelease(&pTmp);
	return hr;
}

// Match a pin by pin direction and connection state.
static HRESULT MatchPin(IPin *pPin, PIN_DIRECTION direction, BOOL bShouldBeConnected, BOOL *pResult)
{
	assert(pResult != nullptr);

	BOOL bMatch = FALSE;
	BOOL bIsConnected = FALSE;

	HRESULT hr = IsPinConnected(pPin, &bIsConnected);
	if (SUCCEEDED(hr))
	{
		if (bIsConnected == bShouldBeConnected)
		{
			hr = IsPinDirection(pPin, direction, &bMatch);
		}
	}

	if (SUCCEEDED(hr))
	{
		*pResult = bMatch;
	}
	return hr;
}

// Return the first unconnected input pin or output pin.
static HRESULT FindUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
	IEnumPins *pEnum = nullptr;
	IPin *pPin = nullptr;
	BOOL bFound = FALSE;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		goto done;
	}

	while (S_OK == pEnum->Next(1, &pPin, nullptr))
	{
		hr = MatchPin(pPin, PinDir, FALSE, &bFound);
		if (FAILED(hr))
		{
			goto done;
		}
		if (bFound)
		{
			*ppPin = pPin;
			(*ppPin)->AddRef();
			break;
		}
		SafeRelease(&pPin);
	}

	if (!bFound)
	{
		hr = VFW_E_NOT_FOUND;
	}

done:
	SafeRelease(&pPin);
	SafeRelease(&pEnum);
	return hr;
}

// Connect output pin to filter.
static HRESULT ConnectFilters(IGraphBuilder *pGraph, IPin *pOut, IBaseFilter *pDest)
{
	IPin *pIn = nullptr;

	// Find an input pin on the downstream filter.
	HRESULT hr = FindUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (SUCCEEDED(hr))
	{
		// Try to connect them.
		hr = pGraph->Connect(pOut, pIn);
		pIn->Release();
	}
	return hr;
}

// http://msdn.microsoft.com/en-us/library/dd375432(v=vs.85).aspx
// Release the format block for a media type.
void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = nullptr;
	}
	if (mt.pUnk != nullptr)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = nullptr;
	}
}

// Delete a media type structure that was allocated on the heap.
void _DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt != nullptr)
	{
		_FreeMediaType(*pmt); 
		CoTaskMemFree(pmt);
	}
}

} // namespace SampleCode

static GUID oDSGetMediaSubType(oSURFACE_FORMAT _Format)
{
	size_t size = oSurfaceFormatGetSize(_Format);
	if (size == 2)
		return (const GUID&)oGUID_MEDIASUBTYPE_RGB565;
	else if (size == 3)
		return (const GUID&)oGUID_MEDIASUBTYPE_RGB24;
	else if (size == 4)
	{
		if (oSurfaceFormatIsAlpha(_Format))
			return (const GUID&)oGUID_MEDIASUBTYPE_ARGB32;
		else
			return (const GUID&)oGUID_MEDIASUBTYPE_RGB32;
	}

	return GUID_NULL;
}

struct oDSSampleGrabberCB : ISampleGrabberCB
{
	// Bridge object to avoid multiple inheritance. Basically this is going to 
	// delegate all functionality back to the specified oDSCamera.
	oDSSampleGrabberCB(struct oDSCamera* _pDSCamera) : pDSCamera(_pDSCamera) {}
	ULONG STDMETHODCALLTYPE AddRef(void) override { return 1; }
	ULONG STDMETHODCALLTYPE Release(void) override { return 1; }
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject) override { return E_NOINTERFACE; }
	HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample* pSample) override { return E_FAIL; }
	HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen) override;
	struct oDSCamera* pDSCamera;
};

struct oDSCamera : public oCamera
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oDSCamera(IGraphBuilder* _pGraphBuilder, IAMStreamConfig* _pStreamConfig, IBaseFilter* _pBaseFilter, const char* _Name, unsigned int _ID, bool* _pSuccess);
	~oDSCamera();

	void GetDesc(DESC* _pDesc) threadsafe override;
	const char* GetName() const threadsafe override;
	unsigned int GetID() const threadsafe override;
	bool FindClosestMatchingMode(const MODE& _ModeToMatch, MODE* _pClosestMatch) threadsafe override;
	bool GetModeList(unsigned int* _pNumModes, MODE* _pModes) threadsafe override;
	float GetFPS() const threadsafe override;
	bool SetMode(const MODE& _Mode) threadsafe override;
	bool SetCapturing(bool _Capturing = true) threadsafe override;
	bool IsCapturing() const threadsafe override;
	bool Map(MAPPED* _pMapped) threadsafe override;
	void Unmap() threadsafe override;

protected:
	friend struct oDSSampleGrabberCB;

	bool BufferCB(double _SampleTime, void* _pBuffer, size_t _SizeofBuffer);
	void ResizeTarget(const MODE& _Mode);
	void DestroyOutput();
	bool RecreateOutput(const MODE& _Mode);
	unsigned int CalculateSourceRowPitch() const;
	bool GetSetModeList(unsigned int* _pNumModes, MODE* _pModes, const MODE* _pNewMode) threadsafe;

	oDSSampleGrabberCB SampleGrabberCB;
	oRef<ISampleGrabber> SampleGrabber;
	oRef<IBaseFilter> VideoInput;
	oRef<IBaseFilter> VideoOutput;
	oRef<IGraphBuilder> GraphBuilder;
	oRef<IAMStreamConfig> StreamConfig;
	oRefCount RefCount;
	DESC Desc;
	oConcurrency::shared_mutex DescMutex;
	oStd::atomic_uint RingBufferReadIndex;
	unsigned int MonotonicCounter;
	unsigned int ID;
	oInitOnce<oStd::uri_string> Name;
	volatile bool Running;

	struct FRAME
	{
		double SampleTime;
		unsigned int Frame;
		std::vector<unsigned char> Data;
	};

	FRAME RingBuffer[8];
};

oDSCamera::oDSCamera(IGraphBuilder* _pGraphBuilder, IAMStreamConfig* _pStreamConfig, IBaseFilter* _pBaseFilter, const char* _Name, unsigned int _ID, bool* _pSuccess)
	: GraphBuilder(_pGraphBuilder)
	, VideoInput(_pBaseFilter)
	, StreamConfig(_pStreamConfig)
	#pragma warning(disable:4355) // warning C4355: 'this' : used in base member initializer list
		, SampleGrabberCB(this)
	#pragma warning(default:4355)
	, RingBufferReadIndex(0)
	, MonotonicCounter(0)
	, Name(_Name)
	, Running(true)
	, ID(_ID)
{
	*_pSuccess = false;

	// Find and set the first mode in the list of supported modes to initialize
	// the ring buffer and put the graph in a RUN state.
	unsigned int nModes = 0;
	if (!GetModeList(&nModes, nullptr))
		return;

	MODE* modes = (MODE*)_alloca(nModes * sizeof(MODE));
	if (!GetModeList(&nModes, modes))
		return;

	*_pSuccess = oDSCamera::SetMode(modes[nModes-1]); // start in the highest-fidelity mode
}

oDSCamera::~oDSCamera()
{
	oRef<IMediaControl> MediaControl;
	oV(GraphBuilder->QueryInterface(IID_PPV_ARGS(&MediaControl)));
	MediaControl->Stop();
}

void oDSCamera::ResizeTarget(const MODE& _Mode)
{
	MonotonicCounter = 0;
	RingBufferReadIndex.exchange(0);
	size_t frameSize = oSurfaceMipCalcSize(_Mode.Format, _Mode.Dimensions);
	oFORI(i, RingBuffer)
	{
		RingBuffer[i].SampleTime = 0.0;
		RingBuffer[i].Data.resize(frameSize);
	}
}

void oDSCamera::DestroyOutput()
{
	SampleGrabber = nullptr;
	if (VideoOutput)
	{
		oV(GraphBuilder->RemoveFilter(VideoOutput));
		VideoOutput = nullptr;
	}
}

bool oDSCamera::RecreateOutput(const MODE& _Mode)
{
	if (GUID_NULL == oDSGetMediaSubType(_Mode.Format))
		return oErrorSetLast(std::errc::invalid_argument, "Unsupported format %s", oStd::as_string(_Mode.Format));

	// Create an output node and attach it to the input
	CoCreateInstance((const GUID&)oGUID_CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&VideoOutput));
	oV(GraphBuilder->AddFilter(VideoOutput, L"Sample Grabber"));
	oRef<IEnumPins> EnumPins;
	if (FAILED(VideoInput->EnumPins(&EnumPins)))
	{
		oWinSetLastError();
		return false;
	}

	bool connectedFilters = false;
	oRef<IPin> Pin;
	while (S_OK == EnumPins->Next(1, &Pin, nullptr))
	{
		if (SUCCEEDED(SampleCode::ConnectFilters(GraphBuilder, Pin, VideoOutput)))
		{
			connectedFilters = true;
			break;
		}
	}
	if (!connectedFilters)
		return oErrorSetLast(std::errc::function_not_supported, "Could not connect VideoOutput filter");

	// Now get it as a SampleGrabber and set up its parameters
	oV(VideoOutput->QueryInterface(IID_PPV_ARGS(&SampleGrabber)));
	oV(SampleGrabber->SetCallback(&SampleGrabberCB, 1));

	AM_MEDIA_TYPE outputType;
	memset(&outputType, 0, sizeof(outputType));
	outputType.majortype = (const GUID&)oGUID_MEDIATYPE_Video;
	outputType.subtype = oDSGetMediaSubType(_Mode.Format);
	if (outputType.subtype == GUID_NULL)
		return oErrorSetLast(std::errc::not_supported, "Unsupported format: %s", oStd::as_string(_Mode.Format));

	oV(SampleGrabber->SetMediaType(&outputType));
	return true;
}

void oDSCamera::GetDesc(DESC* _pDesc) threadsafe
{
	oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(DescMutex);
	*_pDesc = thread_cast<DESC&>(Desc);
}

const char* oDSCamera::GetName() const threadsafe
{
	return *Name;
}

unsigned int oDSCamera::GetID() const threadsafe
{
	return ID;
}

template<typename T> inline T sqDiff(const T& _A, const T& _B)
{
	T t = _A - _B;
	return t * t;
}

static bool operator<(const oCamera::MODE& _This, const oCamera::MODE& _That)
{
	int nBitsR1, nBitsG1, nBitsB1, nBitsA1, nBitsR2, nBitsG2, nBitsB2, nBitsA2;
	oSurfaceGetChannelBitSize(_This.Format, &nBitsR1, &nBitsG1, &nBitsB1, &nBitsA1);
	oSurfaceGetChannelBitSize(_That.Format, &nBitsR2, &nBitsG2, &nBitsB2, &nBitsA2);

	if (nBitsR1 >= nBitsR2)
		return false;

	if (nBitsG1 >= nBitsG2)
		return false;

	if (nBitsB1 >= nBitsG2)
		return false;

	if (nBitsA1 >= nBitsA2)
		return false;

	if (!oSurfaceFormatIsBlockCompressed(_This.Format) && oSurfaceFormatIsBlockCompressed(_That.Format))
		return false;

	size_t thisSize = oSurfaceFormatGetSize(_This.Format);
	size_t thatSize = oSurfaceFormatGetSize(_That.Format);

	if (thisSize < thatSize)
		return true;

	if (thisSize == thatSize)
	{
		if (_This.Dimensions.x < _That.Dimensions.x)
			return true;

		if (_This.Dimensions.x == _This.Dimensions.x)
		{
			if (_This.Dimensions.y < _That.Dimensions.y)
				return true;

			if (_This.Dimensions.y == _That.Dimensions.y)
			{
				if (_This.BitRate < _That.BitRate)
					return true;
			}
		}
	}

	return false;
}

static float Distance(const oCamera::MODE& _ModeToMatch, const oCamera::MODE _TestMode)
{
	static const float kFormatWeight = 0.33f;
	static const float kWidthWeight = 0.33f;
	static const float kHeightWeight = 0.33f;
	static const float kBitRateWeight = 0.0f;

	int nBitsR1, nBitsG1, nBitsB1, nBitsA1, nBitsR2, nBitsG2, nBitsB2, nBitsA2;
	oSurfaceGetChannelBitSize(_ModeToMatch.Format, &nBitsR1, &nBitsG1, &nBitsB1, &nBitsA1);
	oSurfaceGetChannelBitSize(_TestMode.Format, &nBitsR2, &nBitsG2, &nBitsB2, &nBitsA2);

	int FormatDiff = sqDiff(nBitsR1, nBitsR2) + sqDiff(nBitsG1, nBitsG2) + sqDiff(nBitsB1, nBitsB2) + sqDiff(nBitsA1, nBitsA2);
	int2 SizeDiff = sqDiff(_ModeToMatch.Dimensions, _TestMode.Dimensions);
	int BitRateDiff = sqDiff(_ModeToMatch.BitRate, _TestMode.BitRate);

	return kFormatWeight*FormatDiff + kWidthWeight*SizeDiff.x + kHeightWeight*SizeDiff.y + kBitRateWeight*BitRateDiff;
}

bool oDSCamera::FindClosestMatchingMode(const MODE& _ModeToMatch, MODE* _pClosestMatch) threadsafe
{
	unsigned int nModes = 0;
	if (!GetModeList(&nModes, nullptr))
		return false;

	MODE* modes = (MODE*)_alloca(nModes * sizeof(MODE));
	if (!GetModeList(&nModes, modes))
		return false;

	float minDistance = std::numeric_limits<float>::max();
	unsigned int minIndex = oInvalid;
	for (unsigned int i = 0; i < nModes; i++)
	{
		float d = Distance(_ModeToMatch, modes[i]);
		if (d <= minDistance) // <= ensures we use the farthest into the list i.e. the highest quality
		{
			minDistance = d;
			minIndex = i;
		}
	}

	*_pClosestMatch = modes[minIndex];
	return true;
}

bool oDSCamera::GetSetModeList(unsigned int* _pNumModes, MODE* _pModes, const MODE* _pNewMode) threadsafe
{
	if (!_pNumModes && (!_pNewMode || (_pNewMode && !_pModes)))
		return oErrorSetLast(std::errc::invalid_argument);

	*_pNumModes = 0;
	int nCapabilities = 0;
	int sizeofConfigStruct = 0;
	oV(StreamConfig->GetNumberOfCapabilities(&nCapabilities, &sizeofConfigStruct));
	oASSERT(sizeofConfigStruct == sizeof(VIDEO_STREAM_CONFIG_CAPS), "");

	if (!nCapabilities)
		return oErrorSetLast(std::errc::not_supported);

	MODE testMode;
	VIDEO_STREAM_CONFIG_CAPS scc;
	for (int i = 0; i < nCapabilities; i++)
	{
		AM_MEDIA_TYPE* type = nullptr;
		oV(StreamConfig->GetStreamCaps(i, &type, (BYTE*)&scc));
		VIDEOINFOHEADER* pVideoHeader = (VIDEOINFOHEADER*)type->pbFormat;
		if (type->majortype == (const GUID&)oGUID_MEDIATYPE_Video && pVideoHeader->bmiHeader.biWidth > 0 && pVideoHeader->bmiHeader.biHeight > 0)
		{
			oSURFACE_FORMAT format = oGDIGetFormat(pVideoHeader->bmiHeader);

			if (format != oSURFACE_UNKNOWN)
			{
				MODE& m = _pModes ? _pModes[(*_pNumModes)] : testMode;
				m.Format = format;
				m.Dimensions = int2(pVideoHeader->bmiHeader.biWidth, pVideoHeader->bmiHeader.biHeight);
				m.BitRate = static_cast<int>(pVideoHeader->dwBitRate);
				if (_pNewMode && !memcmp(_pNewMode, &m, sizeof(MODE)))
				{
					oV(StreamConfig->SetFormat(type));
					SampleCode::_DeleteMediaType(type);
					return true;
				}

				(*_pNumModes)++;
			}
			
			SampleCode::_DeleteMediaType(type);
		}
	}

	if (_pModes && *_pNumModes)
		std::sort(_pModes, _pModes + *_pNumModes);

	if (_pNewMode)
		return oErrorSetLast(std::errc::not_supported, "Mode not valid %s %dx%d", oStd::as_string(_pNewMode->Format), _pNewMode->Dimensions.x, _pNewMode->Dimensions.y);

	return true;
}

bool oDSCamera::GetModeList(unsigned int* _pNumModes, MODE* _pModes) threadsafe
{
	return GetSetModeList(_pNumModes, _pModes, nullptr);
}

float oDSCamera::GetFPS() const threadsafe
{
	FRAME& f = thread_cast<oDSCamera*>(this)->RingBuffer[RingBufferReadIndex]; // const read and RingBufferReadIndex is updated atomically
	return static_cast<float>(f.Frame / f.SampleTime);
}

bool oDSCamera::SetMode(const MODE& _Mode) threadsafe
{
	if (GUID_NULL == oDSGetMediaSubType(_Mode.Format))
		return oErrorSetLast(std::errc::invalid_argument, "Unsupported format %s", oStd::as_string(_Mode.Format));

	MODE closest;
	if (!FindClosestMatchingMode(_Mode, &closest))
		return false;

	if (memcmp(&closest, &_Mode, sizeof(MODE)))
		return oErrorSetLast(std::errc::not_supported, "Unsupported mode specified: %s %dx%d", oStd::as_string(_Mode.Format), _Mode.Dimensions.x, _Mode.Dimensions.y);

	auto pThis = oLockThis(DescMutex);

	oRef<IMediaControl> MediaControl;
	oV(GraphBuilder->QueryInterface(IID_PPV_ARGS(&MediaControl)));

	// Prevent deadlock in BufferCB, we've got an exclusive above
	bool PriorRunning = Running;
	Running = false;
	if (PriorRunning)
		MediaControl->StopWhenReady();

	pThis->DestroyOutput();
	unsigned int nModes = 0;
	if (!GetSetModeList(&nModes, nullptr, &_Mode))
		return false;

	pThis->ResizeTarget(_Mode);
	if (!pThis->RecreateOutput(_Mode))
		return false;

	pThis->Desc.Mode = _Mode;

	Running = PriorRunning;
	if (Running)
		MediaControl->Run();

	return true;
}

bool oDSCamera::SetCapturing(bool _Capturing) threadsafe
{
	auto pThis = oLockThis(DescMutex);

	oRef<IMediaControl> MediaControl;
	oV(GraphBuilder->QueryInterface(IID_PPV_ARGS(&MediaControl)));

	Running = _Capturing;

	if (_Capturing)
		MediaControl->Run();
	else
		MediaControl->Stop();

	return true;
}

bool oDSCamera::IsCapturing() const threadsafe
{
	return Running;
}

bool oDSCamera::Map(MAPPED* _pMapped) threadsafe
{
	if (Running)
	{
		DescMutex.lock_shared();
		oDSCamera* pThis = thread_cast<oDSCamera*>(this); // protected by Mutex above
		unsigned int index = RingBufferReadIndex;
		
		oSURFACE_DESC d;
		d.Dimensions = int3(thread_cast<DESC&>(Desc).Mode.Dimensions, 1);
		d.Format = Desc.Mode.Format;
		d.Layout = oSURFACE_LAYOUT_IMAGE;
		_pMapped->pData = oStd::data(pThis->RingBuffer[index].Data);
		_pMapped->RowPitch = oSurfaceMipCalcRowPitch(d);
		_pMapped->Frame = pThis->RingBuffer[index].Frame;
	}

	return Running;
}

void oDSCamera::Unmap() threadsafe
{
	DescMutex.unlock_shared();
}

HRESULT oDSSampleGrabberCB::BufferCB(double SampleTime, BYTE* pBuffer, long BufferLen)
{
	return pDSCamera->BufferCB(SampleTime, pBuffer, BufferLen) ? S_OK : E_FAIL;
}

unsigned int oDSCamera::CalculateSourceRowPitch() const
{
	// BITMAPINFOHEADER rules, BITMAPs are aligned to 4 pixels per row when 
	// allocating the image size.
	return oStd::byte_align(Desc.Mode.Dimensions.x, 4) * oSurfaceFormatGetSize(Desc.Mode.Format);
}

bool oDSCamera::BufferCB(double _SampleTime, void* _pBuffer, size_t _SizeofBuffer)
{
	if (Running && _SizeofBuffer > 0)
	{
		oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(DescMutex);

		int WriteIndex = (RingBufferReadIndex + 1) % oCOUNTOF(RingBuffer);
	
		FRAME& f = RingBuffer[WriteIndex];
		oCRTASSERT(_SizeofBuffer == oStd::size(f.Data), "Mismatched buffer size. Got %u bytes, expected %u bytes", _SizeofBuffer, oStd::size(f.Data));
		if (_SizeofBuffer != oStd::size(f.Data))
			return false;
		f.Frame = MonotonicCounter++;
		f.SampleTime = _SampleTime;
		oSURFACE_DESC d;
		d.Dimensions = int3(thread_cast<DESC&>(Desc).Mode.Dimensions, 1);
		d.Format = Desc.Mode.Format;
		d.Layout = oSURFACE_LAYOUT_IMAGE;

		int DestRowPitch = oSurfaceMipCalcRowPitch(d);
		int DestRowSize = oSurfaceMipCalcRowSize(Desc.Mode.Format, Desc.Mode.Dimensions);
		int NumRows = oSurfaceMipCalcNumRows(Desc.Mode.Format, Desc.Mode.Dimensions);
		oMemcpy2dVFlip(oStd::data(f.Data), DestRowPitch, _pBuffer, CalculateSourceRowPitch(), DestRowSize, NumRows);
		RingBufferReadIndex.exchange(WriteIndex);
	}

	return true;
}

static bool oDSCreateCaptureGraphBuilder(ICaptureGraphBuilder2** _ppCaptureGraphBuilder, IGraphBuilder** _ppGraphBuilder)
{
	CoCreateInstance((const GUID&)oGUID_CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(_ppCaptureGraphBuilder));
	CoCreateInstance((const GUID&)oGUID_CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,IID_PPV_ARGS(_ppGraphBuilder));
	(*_ppCaptureGraphBuilder)->SetFiltergraph(*_ppGraphBuilder);
	return true;
}

static bool oDSMonikerEnum(unsigned int _Index, IMoniker** _ppMoniker)
{
	oRef<ICreateDevEnum> DevEnum;
	oVB_RETURN2(CoCreateInstance((const GUID&)oGUID_CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, (const GUID&)oGUID_IID_ICreateDevEnum, (void**)&DevEnum));

	oRef<IEnumMoniker> EnumMoniker;
	HRESULT hr = DevEnum->CreateClassEnumerator((const GUID&)oGUID_CLSID_VideoInputDeviceCategory, &EnumMoniker, 0);
	if (S_OK != hr)
		return oErrorSetLast(std::errc::no_such_device, "No video input devices found%s", oIsWindows64Bit() ? " (might be because of a 32-bit driver on 64-bit Windows)" : "");

	std::vector<oRef<IMoniker> > Monikers;
	Monikers.resize(_Index + 1);

	ULONG nFetched = 0;
	if (FAILED(EnumMoniker->Next(oUInt(Monikers.size()), &Monikers[0], &nFetched)) || (nFetched <= _Index))
		return oErrorSetLast(std::errc::no_such_device);

	*_ppMoniker = Monikers[_Index];
	(*_ppMoniker)->AddRef();

	return true;
}

bool oDSGetStringProperty(char* _StrDestination, size_t _SizeofStrDestination, IMoniker* _pMoniker, const char* _Property)
{
	oRef<IPropertyBag> PropertyBag;
	oVB_RETURN2(_pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)&PropertyBag));
	VARIANT varName;
	VariantInit(&varName);
	oStd::lwstring PropertyName = _Property;
	oVB_RETURN2(PropertyBag->Read(PropertyName.c_str(), &varName, 0));
	oStrcpy(_StrDestination, _SizeofStrDestination, varName.bstrVal);
	VariantClear(&varName);
	return true;
}

bool oDSGetDisplayName(char* _StrDestination, size_t _SizeofStrDestination, IMoniker* _pMoniker)
{
	LPOLESTR lpDisplayName = nullptr;
	oVB_RETURN2(_pMoniker->GetDisplayName(nullptr, nullptr, &lpDisplayName));
	oStrcpy(_StrDestination, _SizeofStrDestination, lpDisplayName);
	oRef<IMalloc> Malloc;
	oVB_RETURN2(CoGetMalloc(1, &Malloc));
	Malloc->Free(lpDisplayName);
	return true;
}

bool oCameraEnum(unsigned int _Index, threadsafe oCamera** _ppCamera)
{
	if (!_ppCamera)
		return oErrorSetLast(std::errc::invalid_argument);

	*_ppCamera = nullptr;

	//if (oIsWindows64Bit())
	//	return oErrorSetLast(std::errc::not_supported, "oCamera is not supported in 64-bit builds because of the lack of valid 64-bit camera drivers or a way of reliably determining if a camera returned uses 32- or 64-bit drivers.");

	oV(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));
	oStd::finally OnScopeExit([&] { CoUninitialize(); });

	oRef<IMoniker> Moniker;
	if (!oDSMonikerEnum(_Index, &Moniker))
		return false;

	oStd::uri_string Name;
	oDSGetStringProperty(Name.c_str(), Name.capacity(), Moniker, "FriendlyName");
	//oStd::mstring Description;
	//oDSGetStringProperty(Description.c_str(), Description.capacity(), Moniker, "Description");
	//oStd::mstring DevicePath;
	//oDSGetStringProperty(DevicePath.c_str(), DevicePath.capacity(), Moniker, "DevicePath");
	// This comes out more consistently than DevicePath, but we use ID below
	//oStd::mstring DisplayName;
	//oVERIFY(oDSGetDisplayName(DisplayName.c_str(), DisplayName.capacity(), Moniker));
	unsigned int MonikerID = 0;
	oV(Moniker->Hash((DWORD*)&MonikerID));

	oRef<IBaseFilter> BaseFilter;
	if (FAILED(Moniker->BindToObject(nullptr, nullptr, (const GUID&)oGUID_IID_IBaseFilter, (void**)&BaseFilter)))
		return oErrorSetLast(std::errc::function_not_supported, "Found device \"%s\" (ID=0x%x), but could not bind an interface for it", Name.c_str(), MonikerID);

	bool running = Moniker->IsRunning(nullptr, nullptr, nullptr) == S_OK;

	oTRACE("Found device \"%s\" (ID=0x%x), continuing...", Name.c_str(), MonikerID);

	oRef<ICaptureGraphBuilder2> CaptureGraphBuilder;
	oRef<IGraphBuilder> GraphBuilder;
	if (!oDSCreateCaptureGraphBuilder(&CaptureGraphBuilder, &GraphBuilder))
		return oWinSetLastError();

	if (SUCCEEDED(GraphBuilder->AddFilter(BaseFilter, L"Video Input")))
	{
		oRef<IAMStreamConfig> AMStreamConfig;
		if (SUCCEEDED(CaptureGraphBuilder->FindInterface(nullptr, (const GUID*)&oGUID_MEDIATYPE_Video, BaseFilter, (const GUID&)oGUID_IID_IAMStreamConfig, (void**)&AMStreamConfig)))
		{
			// @oooii-tony: At this point there is a camera. A camera most likely can
			// support several surface descs, so that's a mode to set on the camera.

			bool success = false;
			oCONSTRUCT(_ppCamera, oDSCamera(GraphBuilder, AMStreamConfig, BaseFilter, Name, MonikerID, &success));
		}
	}

	return !!*_ppCamera;
}
