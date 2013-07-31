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
// Does not always detect multiple gpu's at the moment. win32 function EnumDisplaySettings does not succeed sometimes, even for successfully enumed displays(called in oDisplay.cpp)
#include <oStd/oStdMakeUnique.h>
#include <oPlatform/oTest.h>
#include <oPlatform/Windows/oDXGI.h>
#include <oPlatform/oProgressBar.h>
#include <oPlatform/oStandards.h>
#include <oPlatform/oConsole.h>
#include <oPlatform/oDisplay.h>
#include <oGPU/oGPUWindow.h> // @oooii-tony: Because this is here, we should move this test to at least oGPU_ (out of oPlatform)
#include <oGfx/oGfxMosaic.h>
#include <oConcurrency/mutex.h>

#include "oHLSLPCIEBandwidthVS4ByteCode.h"
#include "oHLSLPCIEBandwidthPS4ByteCode.h"

static const char* ReportTableFormatString = "%-30s%-12s%-16s%-12s%-12s%-8s\n";

static const int NUM_FRAMES = 100;

enum TEXTURE_COPY_METHOD
{
	DEFERRED_MODE		= 0,
	IMMEDIATE_MODE		= 16,
	METHOD_MASK			= 0xf,

	MAP_UNMAP			= 1,
	UPDATE_RESOURCE		= 2,
	MAP_UNMAP_IMM		= MAP_UNMAP | IMMEDIATE_MODE,
	UPDATE_RESOURCE_IMM	= UPDATE_RESOURCE | IMMEDIATE_MODE,
};

class Test
{
public:
	Test(int _width, int _height, int _numTextures, oSURFACE_FORMAT _format, TEXTURE_COPY_METHOD _method) : Width(_width), 
		Height(_height), NumTextures(_numTextures), Format(_format), Method(_method)
	{
		FrameSize = oSurfaceMipCalcSize(_format, int2(_width, _height));
		if((Method & METHOD_MASK) == UPDATE_RESOURCE)
		{
			TextureData.resize(FrameSize * _numTextures);
			oSURFACE_DESC surfaceDesc;
			surfaceDesc.Dimensions = int3(Width, Height, 1);
			surfaceDesc.Format = Format;

			for (int i = 0;i < NumTextures; ++i)
			{
				oSURFACE_MAPPED_SUBRESOURCE data;
				data.pData = oStd::byte_add(oStd::data(TextureData), FrameSize*i);
				data.RowPitch = oSurfaceMipCalcRowPitch(surfaceDesc);
				data.DepthPitch = oSurfaceMipCalcDepthPitch(surfaceDesc);
				SubResourceData.push_back(data);
			}
		}
		FrameSize *= _numTextures;
	}

	Test(Test&& _other) : Width(_other.Width), Height(_other.Height), Format(_other.Format), NumTextures(_other.NumTextures),
		FrameSize(_other.FrameSize), Method(_other.Method), TextureData(std::move(_other.TextureData)),
		SubResourceData(std::move(_other.SubResourceData))
	{
	}

	oSURFACE_MAPPED_SUBRESOURCE* GetTextureSourceData(int _texture)
	{
		if((Method & METHOD_MASK) == UPDATE_RESOURCE)
			return &SubResourceData[_texture];
		else
			return nullptr;
	}

	void Finish(double _testTime, oInt _numWindows)
	{
		double dataRate = ((((double)FrameSize.Ref()) * NUM_FRAMES)/_testTime)*_numWindows.Ref();
		double fps = NUM_FRAMES / _testTime;
		oStd::sstring dataRateString1, dataRateString2;
		oPrintf(dataRateString2, "%s/s", oFormatMemorySize(dataRateString1, static_cast<long long>(dataRate), 2));
		oStd::sstring method, format;
		oStd::sstring size;
		oPrintf(size, "%dx%dx%d", Width, Height, NumTextures);
		oStd::sstring fpsString;
		oPrintf(fpsString, "%0.2f", fps);
		oStd::sstring numWindowsString;
		oPrintf(numWindowsString, "%d", _numWindows.Ref());
		oPrintf(ResultsString, ReportTableFormatString, oStd::to_string(method, Method), numWindowsString, oStd::to_string(format, Format), size, dataRateString2, fpsString);
	}

	oStd::mstring ResultsString;
	int Width;
	int Height;
	oSURFACE_FORMAT Format;
	int NumTextures;
	oInt FrameSize;
	TEXTURE_COPY_METHOD Method;
	std::vector<char> TextureData; //Used to pass to create a texture for "RECREATE" method.
private:
	Test(const Test& _other);
	Test& operator=(const Test& _other);

	std::vector<oSURFACE_MAPPED_SUBRESOURCE> SubResourceData;
};

struct GlobalSettings //setting shared by every window
{
	GlobalSettings() 
		: CurrentTest(-1)
		, ClientSize(640, 480)
	{
	}

	int CurrentTest;
	int2 ClientSize;

	oConcurrency::shared_mutex ChangeTestLock;

	std::vector<Test> Tests;
};

static const oGPU_VERTEX_ELEMENT sVEMosaic[] = 
{
	{ 'POS0', oSURFACE_R32G32B32_FLOAT, 0, false, },
	{ 'TEX0', oSURFACE_R32G32_FLOAT, 0, false, },
};

struct TESTWindow
{
	oRef<threadsafe oGPUWindow> Window;
	oRef<oGPUCommandList> CommandList;
	int HookID;
	oRef<oGfxMosaic> Mosaic;
	std::vector<oRef<oGPUTexture>> Textures;
	GlobalSettings& Settings;
	int FrameCount;

	TESTWindow(GlobalSettings& _settings, unsigned int _Adapter, int2 _Position)
		: Settings(_settings)
	{
#if 1
		oGPUDevice::INIT init;
		init.DebugName = "TESTPCIEBandwidth Device";
		init.Version = oVersion(11,0);
		init.AdapterIndex = _Adapter - 1;
		
		oRef<oGPUDevice> Device;
		oVERIFY(oGPUDeviceCreate(init, &Device));

		oGPU_WINDOW_INIT WinInit;
		WinInit.WindowTitle = "TESTPCIEBandwidth";
		WinInit.VSynced = false;
		WinInit.EventHook = oBIND(&TESTWindow::EventHook, this, oBIND1);
		WinInit.RenderFunction = oBIND(&TESTWindow::Render, this, oBIND1);

		WinInit.WinDesc.ClientSize = Settings.ClientSize;
		WinInit.WinDesc.ClientPosition = _Position;
		//oConsoleReporting::Report(oConsoleReporting::INFO, "Position %d %d\n", _Position.x, _Position.y);
		WinInit.WinDesc.Debug = false;
		WinInit.InitialAlignment = oGUI_ALIGNMENT_TOP_LEFT;

		if(!oGPUWindowCreate(WinInit, Device, &Window))
			return;

		oGPU_PIPELINE_DESC pd;
		pd.InputType = oGPU_TRIANGLES;
		pd.pElements = sVEMosaic;
		pd.NumElements = oCOUNTOF(sVEMosaic);
		pd.pVertexShader = oHLSLPCIEBandwidthVS4ByteCode;
		pd.pPixelShader = oHLSLPCIEBandwidthPS4ByteCode;
		if (!oGfxMosaicCreate(Device, pd, &Mosaic))
			return;

		oGPUCommandList::DESC clDesc;
		clDesc.DrawOrder = 0;

		if (!Device->CreateCommandList("TestCL", clDesc, &CommandList))
			return;

#endif
	}

	bool EventHook(const oGUI_EVENT_DESC& _Event)
	{
		return true;
	}

	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		int LocalCurrentTest = Settings.CurrentTest;
		if (LocalCurrentTest == -1)
			return;

		if (FrameCount < NUM_FRAMES)
		{
			oConcurrency::shared_lock<oConcurrency::shared_mutex> lock(Settings.ChangeTestLock);

			oRef<oGPUDevice> Device;
			oVERIFY(Window->QueryInterface(&Device));

			oRef<oGPUCommandList> CurrentCommandList;

			if (Settings.Tests[LocalCurrentTest].Method & IMMEDIATE_MODE)
				Device->GetImmediateCommandList(&CurrentCommandList);
			else
				CurrentCommandList = CommandList;

			CurrentCommandList->Begin();
			CurrentCommandList->SetRenderTarget(_pPrimaryRenderTarget);
			//CurrentCommandList->Clear(oGPU_CLEAR_COLOR);

			if((Settings.Tests[LocalCurrentTest].Method & METHOD_MASK) == UPDATE_RESOURCE)
			{
				for (int i = 0; i < oInt(Textures.size()); ++i)
				{
					oSURFACE_BOX dstRect;
					dstRect.Right = Settings.Tests[LocalCurrentTest].Width;
					dstRect.Bottom = Settings.Tests[LocalCurrentTest].Height;

					auto subResourceData = Settings.Tests[LocalCurrentTest].GetTextureSourceData(i);
					CurrentCommandList->Commit(Textures[i], 0, *subResourceData, dstRect);
				}
			}
			else
			{
				for (int i = 0; i < oInt(Textures.size()); ++i)
				{
					oSURFACE_MAPPED_SUBRESOURCE mapped;
					CurrentCommandList->Reserve(Textures[i], 0, &mapped);
					//oMemset2d(mapped.pData, mapped.RowPitch, 255, mapped.RowPitch, Settings.Tests[LocalCurrentTest].Height);
					CurrentCommandList->Commit(Textures[i], 0, mapped);
				}
			}

			for (int i = 0; i < oInt(Textures.size()); ++i)
				CurrentCommandList->SetShaderResources(i, 1, &Textures[i]);

			Mosaic->Draw(CurrentCommandList, _pPrimaryRenderTarget, 0, oUInt(Textures.size()), &Textures[0]);

			CurrentCommandList->End();

			FrameCount++;
		}
	}

	bool CreateTextureSet(Test& _test, bool _bSetup = true)
	{
		Textures.clear();

		oRef<oGPUDevice> Device;
		if (!Window->QueryInterface(&Device))
			return oErrorSetLast(std::errc::invalid_argument, "Could not get d3d10 device from window");

		Textures.resize(_test.NumTextures);

		for (int i = 0;i < _test.NumTextures; ++i)
		{
			oGPUTexture::DESC texDesc;
			texDesc.Dimensions = int3(_test.Width, _test.Height, 1);
			texDesc.ArraySize = 1;
			texDesc.Format = _test.Format;
			texDesc.Type = oGPU_TEXTURE_2D_MAP;

			if (!Device->CreateTexture("test", texDesc, &Textures[i]))
				return false;
		}

		if (_bSetup)
		{
			oGeometryFactory::MOSAIC_DESC MosaicDesc;
			MosaicDesc.SourceSize = int2(_test.Width, _test.Height);
			MosaicDesc.SourceTexelSpace = oRECT(oRECT::pos_size, int2(0,0), int2(_test.Width, _test.Height));
			MosaicDesc.DestinationSize = Settings.ClientSize;
			MosaicDesc.pSourceRects = nullptr;
			MosaicDesc.pDestinationRects = nullptr;
			MosaicDesc.NumRectangles = 0;
			MosaicDesc.FaceType = oGeometry::FRONT_CW;
			MosaicDesc.ZPosition = 0.0f; // currently ignored by shader
			MosaicDesc.FlipTexcoordV = false;
			MosaicDesc.FlipPositionY = true;

			oVERIFY(Mosaic->Rebuild(MosaicDesc));
		}
		return true;
	}

	bool IsFinished() const { return FrameCount >= NUM_FRAMES; }
	void ResetFrameCount() { FrameCount = 0; }
};

struct PLATFORM_PCIEBandwidth : public oTest
{	
	GlobalSettings Settings;
	std::vector<std::unique_ptr<TESTWindow>> TestWindows;

	PLATFORM_PCIEBandwidth()
	{
		oConsole::Clear();
		oConsoleReporting::Report(oConsoleReporting::INFO, "\n");
		oConsoleReporting::Report(oConsoleReporting::INFO, "\n");

		Settings.Tests.push_back(Test(4096, 4096, 1, oSURFACE_R8G8B8A8_UNORM, MAP_UNMAP));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8_UNORM, MAP_UNMAP));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8G8_UNORM, MAP_UNMAP));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC4_UNORM, MAP_UNMAP));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC5_UNORM, MAP_UNMAP));
		Settings.Tests.push_back(Test(4096, 4096, 1, oSURFACE_R8G8B8A8_UNORM, UPDATE_RESOURCE));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8_UNORM, UPDATE_RESOURCE));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8G8_UNORM, UPDATE_RESOURCE));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC4_UNORM, UPDATE_RESOURCE));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC5_UNORM, UPDATE_RESOURCE));
		Settings.Tests.push_back(Test(4096, 4096, 1, oSURFACE_R8G8B8A8_UNORM, MAP_UNMAP_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8_UNORM, MAP_UNMAP_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8G8_UNORM, MAP_UNMAP_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC4_UNORM, MAP_UNMAP_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC5_UNORM, MAP_UNMAP_IMM));
		Settings.Tests.push_back(Test(4096, 4096, 1, oSURFACE_R8G8B8A8_UNORM, UPDATE_RESOURCE_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8_UNORM, UPDATE_RESOURCE_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_R8G8_UNORM, UPDATE_RESOURCE_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC4_UNORM, UPDATE_RESOURCE_IMM));
		Settings.Tests.push_back(Test(8192, 8192, 1, oSURFACE_BC5_UNORM, UPDATE_RESOURCE_IMM));
		
		std::unordered_map<HMONITOR, oDISPLAY_DESC> displays;
		int numDisplays = oDisplayGetNum();
		for (int i = 0;i < numDisplays; ++i)
		{
			oDISPLAY_DESC displayDesc;
			displayDesc.Index = -1;
			oDisplayEnum(i, &displayDesc);
			oConsoleReporting::Report(oConsoleReporting::INFO, "Position %d %d Position2 %d %d hmon %p index %d\n", displayDesc.Position.x, displayDesc.Position.y, displayDesc.WorkareaPosition.x,
				displayDesc.WorkareaPosition.y, displayDesc.NativeHandle, displayDesc.Index);
			displays[(HMONITOR)displayDesc.NativeHandle] = displayDesc;
		}

		oConsoleReporting::Report(oConsoleReporting::INFO, "\n");
		oConsoleReporting::Report(oConsoleReporting::INFO, "\n");

		std::unordered_map<unsigned int, int2> adapters; //id of adapter, and the position to place a window on it.
		oDXGIEnumOutputs([&](unsigned int _AdapterIndex, IDXGIAdapter* _pAdapter, unsigned int _OutputIndex, IDXGIOutput* _pOutput) -> bool{
			DXGI_OUTPUT_DESC desc;
			_pOutput->GetDesc(&desc);
			if(desc.AttachedToDesktop)
			{
				oConsoleReporting::Report(oConsoleReporting::INFO, "Mon %p Adapter %d Output %d\n", (void*)desc.Monitor, _AdapterIndex, _OutputIndex);
				if(adapters.find(_AdapterIndex) == std::end(adapters))
				{
					auto display = displays.find(desc.Monitor);
					if(display != std::end(displays))
					{
						oDISPLAY_DESC& displayDesc = display->second;

						int2 pos;
						pos.x = (desc.DesktopCoordinates.right - desc.DesktopCoordinates.left)/2 - Settings.ClientSize.x/2 + displayDesc.Position.x;
						pos.y = (desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top)/2 - Settings.ClientSize.y/2 + displayDesc.Position.y;

						adapters[_AdapterIndex] = pos;
					}
				}
			}
			return true;
		});

		oConsoleReporting::Report(oConsoleReporting::INFO, "\n");
		oConsoleReporting::Report(oConsoleReporting::INFO, "\n");

		oFOR(auto& _entry, adapters)
		{
			TestWindows.push_back(oStd::make_unique<TESTWindow>(Settings, _entry.first, _entry.second));
		}
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{	
#if 1
		oConsoleReporting::Report(oConsoleReporting::INFO,"\n");
		oConsoleReporting::Report(oConsoleReporting::INFO, ReportTableFormatString, "Method:", "NumWindows:", "Format:", "Size:", "Bandwidth:", "FPS:");

		oSleep(500); //let oWindow stabilize for a half sec before starting.

		oProgressBar::DESC pb;
		pb.Show = true;
		pb.ShowStopButton = true;
		pb.AlwaysOnTop = true;
		pb.UnknownProgress = false;
		oRef<threadsafe oProgressBar> ProgressBar;
		oVERIFY(oProgressBarCreate(pb, nullptr, &ProgressBar));
		ProgressBar->SetTitle("TESTPCIEBandwith Progress");

		int TotalTestFrames = oInt(NUM_FRAMES * Settings.Tests.size());
		int progress = 0;

		for (unsigned int t = 0;t < Settings.Tests.size(); ++t)
		{
			auto& _test = Settings.Tests[t];
			{
				oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Settings.ChangeTestLock);
				Settings.CurrentTest = oInt(t);
				for (size_t j = 0;j < TestWindows.size(); ++j)
				{
					oTESTB(TestWindows[j]->CreateTextureSet(_test), "Failed to create textures for a test");
				}
			}

			double startTime = oTimer();
			std::for_each(begin(TestWindows), end(TestWindows), oBIND(&TESTWindow::ResetFrameCount, oBIND1));
			
			int LastFrameCount = oInvalid;
			bool finished = false;
			while (!finished)
			{
				if (LastFrameCount != TestWindows[0]->FrameCount)
				{
					ProgressBar->SetPercentage(((TestWindows[0]->FrameCount + (t*NUM_FRAMES))*100)/TotalTestFrames);
					LastFrameCount = TestWindows[0]->FrameCount;
				}

				oProgressBar::DESC PBDesc;
				ProgressBar->GetDesc(&PBDesc);
				if (PBDesc.Stopped)
				{
					Settings.CurrentTest = -1;
					for (size_t j = 0;j < TestWindows.size(); ++j)
					{
						TestWindows[j]->Window->Close();
					}
					oPrintf(_StrStatus, _SizeofStrStatus, "Canceled");
					return SUCCESS;
				}
				finished = std::all_of(begin(TestWindows), end(TestWindows), oBIND(&TESTWindow::IsFinished, oBIND1));
				if(!finished)
					oSleep(10);
			}
			
			double totalTime = oTimer() - startTime;

			{
				oConcurrency::lock_guard<oConcurrency::shared_mutex> lock(Settings.ChangeTestLock);
				_test.Finish(totalTime, TestWindows.size());
				oConsoleReporting::Report(oConsoleReporting::INFO,"%s", Settings.Tests[t].ResultsString);
			}
		}

		Settings.CurrentTest = -1;

		for (size_t j = 0;j < TestWindows.size(); ++j)
		{
			TestWindows[j]->Window->Close();
		}
		TestWindows.clear();
#endif
		return SUCCESS;
	}
};

namespace oStd {
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const TEXTURE_COPY_METHOD& _Value)
{
	switch (_Value)
	{
	case MAP_UNMAP:
		oPrintf(_StrDestination, _SizeofStrDestination, "Map / UnMap Deferred");
		break;
	case UPDATE_RESOURCE:
		oPrintf(_StrDestination, _SizeofStrDestination, "UpdateSubresource Deferred");
		break;
	case MAP_UNMAP_IMM:
		oPrintf(_StrDestination, _SizeofStrDestination, "Map / UnMap Immediate");
		break;
	case UPDATE_RESOURCE_IMM:
		oPrintf(_StrDestination, _SizeofStrDestination, "UpdateSubresource Immediate");
		break;
	oNODEFAULT;
	}
	return _StrDestination;
}

} // namespace oStd

oTEST_REGISTER_PERFTEST(PLATFORM_PCIEBandwidth);
