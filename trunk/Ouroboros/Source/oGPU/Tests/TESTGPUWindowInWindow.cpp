/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oPlatform/oTest.h>
#include <oPlatform/oWindow.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oGPU/oGPUWindow.h> // @oooii-tony: Because this is here, we should move this test to at least oGPU (out of oPlatform)

static const bool kInteractiveMode = false;

class WindowInWindow
{
public:
	WindowInWindow(bool* _pSuccess)
		: Counter(0)
	{
		*_pSuccess = false;

		// Create the parent:

		oWINDOW_INIT init;
		init.WindowThreadDebugName = "Parent WinThread";
		init.WindowTitle = "Window-In-Window Test";
		init.EventHook = oBIND(&WindowInWindow::ParentEventHook, this, oBIND1);
		init.WinDesc.Style = oGUI_WINDOW_SIZEABLE;
		init.WinDesc.ClientSize = int2(640,480);
		init.WinDesc.Debug = true;
		init.WinDesc.HasFocus = false;

		if (!oWindowCreate(init, &ParentWindow))
			return; // pass through error

		oGPUDevice::INIT DevInit;
		DevInit.DebugName = "TestDevice";
		DevInit.Version = oVersion(10,0);
		DevInit.EnableDebugReporting = true;

		oRef<oGPUDevice> Device;
		if (!oGPUDeviceCreate(DevInit, &Device))
			return; // pass through error

		oGPUCommandList::DESC CLDesc;
		CLDesc.DrawOrder = 0;
		if (!Device->CreateCommandList("TestCL", CLDesc, &CommandList))
			return; // pass through error

		oGUI_WINDOW hWnd = nullptr;
		ParentWindow->QueryInterface(oGetGUID<oGUI_WINDOW>(), &hWnd);

		oGPU_WINDOW_INIT GPUInit;
		GPUInit.WinDesc.hParent = hWnd;
		GPUInit.WinDesc.Style = oGUI_WINDOW_BORDERLESS;
		GPUInit.WinDesc.ClientPosition = int2(20,20);
		GPUInit.WinDesc.ClientSize = int2(600,480-65);
		GPUInit.WinDesc.AllowAltEnter = false;
		GPUInit.WinDesc.DefaultEraseBackground = false;
		GPUInit.WinDesc.Debug = true;
		GPUInit.WinDesc.HasFocus = false;
		GPUInit.InitialAlignment = oGUI_ALIGNMENT_TOP_LEFT;
		GPUInit.EventHook = oBIND(&WindowInWindow::GPUWindowEventHook, this, oBIND1);
		GPUInit.RenderFunction = oBIND(&WindowInWindow::Render, this, oBIND1);

		if (!oGPUWindowCreate(GPUInit, Device, &GPUWindow))
			return; // pass through error

		*_pSuccess = true;
	}

	~WindowInWindow()
	{
		// Right now the resources have to be explicitly destroyed in reverse order
		// of their creation. This is because the ParentWindow isn't aware of any
		// connection to the GPUWindow other than through the underlying platform
		// HWND. Also, these have to be destroyed before WindowInWindow because the
		// resources queried - namely the device and swap chains - if invalidated 
		// cause the RenderFunction to break because there's no testing in that 
		// function for valid resources.
		// It would be nice to make this more automatic...
		GPUWindow = nullptr;
		ParentWindow = nullptr;
	}

	void Render(oGPURenderTarget* _pPrimaryRenderTarget)
	{
		if (GPUWindow)
		{
			oRef<oGPUDevice> Device;
			GPUWindow->GetDevice(&Device);

			oGPU_CLEAR_DESC CD;
			CD.ClearColor[0] = (Counter & 0x1) ? std::White : std::Blue;
			_pPrimaryRenderTarget->SetClearDesc(CD);

			if (Device->BeginFrame())
			{
				CommandList->Begin();
				CommandList->SetRenderTarget(_pPrimaryRenderTarget);
				CommandList->Clear(_pPrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
				CommandList->End();
				Device->EndFrame();
			}
		}
	}

	bool ParentEventHook(const oGUI_EVENT_DESC& _Event)
	{
		switch (_Event.Event)
		{
			case oGUI_CREATING:
			{
				oGUI_CONTROL_DESC ButtonDesc;
				ButtonDesc.hParent = _Event.hSource;
				ButtonDesc.Type = oGUI_CONTROL_BUTTON;
				ButtonDesc.Text = "Push Me";
				ButtonDesc.Size = int2(100,25);
				ButtonDesc.Position = int2(10,480-10-ButtonDesc.Size.y);
				ButtonDesc.ID = 0;
				ButtonDesc.StartsNewGroup = false;
				hButton = oWinControlCreate(ButtonDesc);
				break;
			}

			case oGUI_CLOSED:
				GPUWindow = nullptr;
				break;

			case oGUI_SIZED:
			{
				if (GPUWindow)
				{
					oGUI_WINDOW_DESC* d = nullptr;
					GPUWindow->Map(&d);
					d->ClientSize = _Event.ClientSize - int2(40,65);
					GPUWindow->Unmap();
				}
				SetWindowPos(hButton, 0, 10, _Event.ClientSize.y-10-25, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOZORDER);
				break;
			}

			default:
				break;
		}

		return true;
	}

	bool GPUWindowEventHook(const oGUI_EVENT_DESC& _Event)
	{
		return true;
	}

	threadsafe oWindow* GetWindow() threadsafe { return ParentWindow; }

	void IncrementClearCounter() { Counter++; }

private:
	oRef<threadsafe oWindow> ParentWindow;
	oRef<threadsafe oGPUWindow> GPUWindow;
	oRef<oGPUCommandList> CommandList;

	HWND hButton;
	int Counter;
};

struct TESTGPUWindowInWindow : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		bool success = false;
		WindowInWindow test(&success);
		oTESTB0(success);

		if (kInteractiveMode)
		{
			while (test.GetWindow()->IsOpen())
			{
				oSleep(1000);
				test.IncrementClearCounter();
			}
		}

		else
		{
			oSleep(200);
			oStd::future<oRef<oImage>> snapshot = test.GetWindow()->CreateSnapshot();
			oTESTFI(snapshot);
			test.IncrementClearCounter();

			oSleep(200);
			snapshot = test.GetWindow()->CreateSnapshot();
			oTESTFI2(snapshot, 1);
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTGPUWindowInWindow);
