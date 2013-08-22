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
#include <oPlatform/oTest.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oWindow.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oGPU/oGPU.h>

static const bool kInteractiveMode = false;

class WindowInWindow
{
public:
	WindowInWindow(bool* _pSuccess)
		: Counter(0)
		, Running(true)
	{
		*_pSuccess = false;

		// Create the parent:

		{
			oWINDOW_INIT init;
			init.Title = "Window-In-Window Test";
			init.EventHook = oBIND(&WindowInWindow::ParentEventHook, this, oBIND1);
			init.Shape.State = oGUI_WINDOW_HIDDEN;
			init.Shape.Style = oGUI_WINDOW_SIZABLE;
			init.Shape.ClientSize = int2(640, 480);

			if (!oWindowCreate(init, &ParentWindow))
				return; // pass through error
		}

		oGPUDevice::INIT DevInit;
		DevInit.DebugName = "TestDevice";
		DevInit.Version = oVersion(10, 0);
		DevInit.DriverDebugLevel = oGPU_DEBUG_NORMAL;

		if (!oGPUDeviceCreate(DevInit, &Device))
			return; // pass through error

		oGPUCommandList::DESC CLDesc;
		CLDesc.DrawOrder = 0;
		if (!Device->CreateCommandList("TestCL", CLDesc, &CommandList))
			return; // pass through error

		{
			oWINDOW_INIT init;
			init.Shape.State = oGUI_WINDOW_RESTORED;
			init.Shape.Style = oGUI_WINDOW_BORDERLESS;
			init.Shape.ClientPosition = int2(20,20);
			init.Shape.ClientSize = int2(600,480-65);
			init.EventHook = oBIND(&WindowInWindow::GPUWindowEventHook, this, oBIND1);

			if (!oWindowCreate(init, &GPUWindow))
				return; // pass through error

			Device->CreatePrimaryRenderTarget(GPUWindow, oSURFACE_UNKNOWN, true, &PrimaryRenderTarget);
			GPUWindow->SetParent(ParentWindow);
		}

		ParentWindow->Show();
		*_pSuccess = true;
	}

	inline bool IsRunning() const { return Running; }

	void Render()
	{
		if (PrimaryRenderTarget)
		{
			if (Device->BeginFrame())
			{
				oGPU_CLEAR_DESC CD;
				CD.ClearColor[0] = (Counter & 0x1) ? oStd::White : oStd::Blue;
				PrimaryRenderTarget->SetClearDesc(CD);

				CommandList->Begin();
				CommandList->SetRenderTarget(PrimaryRenderTarget);
				CommandList->Clear(PrimaryRenderTarget, oGPU_CLEAR_COLOR_DEPTH_STENCIL);
				CommandList->End();

				Device->EndFrame();
				Device->Present(1);
			}
		}
	}

	void ParentEventHook(const oGUI_EVENT_DESC& _Event)
	{
		switch (_Event.Type)
		{
			case oGUI_CREATING:
			{
				oGUI_CONTROL_DESC ButtonDesc;
				ButtonDesc.hParent = _Event.hWindow;
				ButtonDesc.Type = oGUI_CONTROL_BUTTON;
				ButtonDesc.Text = "Push Me";
				ButtonDesc.Size = int2(100,25);
				ButtonDesc.Position = int2(10,480-10-ButtonDesc.Size.y);
				ButtonDesc.ID = 0;
				ButtonDesc.StartsNewGroup = false;
				hButton = oWinControlCreate(ButtonDesc);
				break;
			}

			case oGUI_CLOSING:
				Running = false;
				break;

			case oGUI_SIZED:
			{
				if (GPUWindow)
					GPUWindow->SetClientSize(_Event.AsShape().Shape.ClientSize - int2(40,65));

				if (PrimaryRenderTarget)
					PrimaryRenderTarget->Resize(int3(_Event.AsShape().Shape.ClientSize - int2(40,65), 1));

				SetWindowPos(hButton, 0, 10, _Event.AsShape().Shape.ClientSize.y-10-25, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOZORDER);
				break;
			}

			default:
				break;
		}
	}

	void GPUWindowEventHook(const oGUI_EVENT_DESC& _Event)
	{
	}

	oWindow* GetWindow() threadsafe { return ParentWindow; }

	void FlushMessages()
	{
		GPUWindow->FlushMessages();
		ParentWindow->FlushMessages();
	}

	void IncrementClearCounter() { Counter++; }

private:
	oRef<oGPUDevice> Device;
	oRef<oWindow> ParentWindow;
	oRef<oWindow> GPUWindow;
	oRef<oGPUCommandList> CommandList;
	oRef<oGPURenderTarget> PrimaryRenderTarget;

	HWND hButton;
	int Counter;
	bool Running;
};

struct GPU_WindowInWindow : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		if (oSystemIsRemote())
		{
			oPrintf(_StrStatus, _SizeofStrStatus, "Detected remote session: differing text anti-aliasing will cause bad image compares");
			return SKIPPED;
		}

		// Turn display power on, otherwise the test will fail
		oDisplaySetPowerOn(true);

		bool success = false;
		WindowInWindow test(&success);
		oTESTB0(success);

		if (kInteractiveMode)
		{
			while (test.IsRunning())
			{
				test.FlushMessages();

				oSleep(1000);
				test.IncrementClearCounter();

				test.Render();
			}
		}

		else
		{
			test.FlushMessages();
			test.Render();
			oStd::future<oRef<oImage>> snapshot = test.GetWindow()->CreateSnapshot();
			while (!snapshot.is_ready()) { test.FlushMessages(); }
			oTESTFI(snapshot);
			test.IncrementClearCounter();
			test.FlushMessages();
			test.Render();
			snapshot = test.GetWindow()->CreateSnapshot();
			while (!snapshot.is_ready()) { test.FlushMessages(); }
			oTESTFI2(snapshot, 1);
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_WindowInWindow);
