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
#include <oGUI/window.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/system.h>
#include <oGPU/oGPU.h>

using namespace ouro;

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
			window::init i;
			i.title = "Window-In-Window Test";
			i.on_event = std::bind(&WindowInWindow::ParentEventHook, this, oBIND1);
			i.shape.state = ouro::window_state::hidden;
			i.shape.style = ouro::window_style::sizable;
			i.shape.client_size = int2(640, 480);
			try { ParentWindow = window::make(i); }
			catch (std::exception& e) { oErrorSetLast(e); return; }
		}

		oGPUDevice::INIT DevInit;
		DevInit.DebugName = "TestDevice";
		DevInit.Version = version(10, 0);
		DevInit.DriverDebugLevel = oGPU_DEBUG_NORMAL;

		if (!oGPUDeviceCreate(DevInit, &Device))
			return; // pass through error

		oGPUCommandList::DESC CLDesc;
		CLDesc.DrawOrder = 0;
		if (!Device->CreateCommandList("TestCL", CLDesc, &CommandList))
			return; // pass through error

		{
			window::init i;
			i.shape.state = ouro::window_state::restored;
			i.shape.style = ouro::window_style::borderless;
			i.shape.client_position = int2(20,20);
			i.shape.client_size = int2(600,480-65);
			i.on_event = std::bind(&WindowInWindow::GPUWindowEventHook, this, std::placeholders::_1);

			try { GPUWindow = window::make(i); }
			catch (std::exception& e) { oErrorSetLast(e); return; }
			
			Device->CreatePrimaryRenderTarget(GPUWindow.get(), surface::unknown, true, &PrimaryRenderTarget);
			GPUWindow->parent(ParentWindow);
		}

		ParentWindow->show();
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
				CD.ClearColor[0] = (Counter & 0x1) ? White : Blue;
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

	void ParentEventHook(const window::basic_event& _Event)
	{
		switch (_Event.type)
		{
			case ouro::gui_event::creating:
			{
				oGUI_CONTROL_DESC ButtonDesc;
				ButtonDesc.hParent = _Event.window;
				ButtonDesc.Type = ouro::control_type::button;
				ButtonDesc.Text = "Push Me";
				ButtonDesc.Size = int2(100,25);
				ButtonDesc.Position = int2(10,480-10-ButtonDesc.Size.y);
				ButtonDesc.ID = 0;
				ButtonDesc.StartsNewGroup = false;
				hButton = oWinControlCreate(ButtonDesc);
				break;
			}

			case ouro::gui_event::closing:
				Running = false;
				break;

			case ouro::gui_event::sized:
			{
				if (GPUWindow)
					GPUWindow->client_size(_Event.as_shape().shape.client_size - int2(40,65));

				if (PrimaryRenderTarget)
					PrimaryRenderTarget->Resize(int3(_Event.as_shape().shape.client_size - int2(40,65), 1));

				SetWindowPos(hButton, 0, 10, _Event.as_shape().shape.client_size.y-10-25, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOZORDER);
				break;
			}

			default:
				break;
		}
	}

	void GPUWindowEventHook(const window::basic_event& _Event)
	{
	}

	window* GetWindow() { return ParentWindow.get(); }

	void flush_messages()
	{
		GPUWindow->flush_messages();
		ParentWindow->flush_messages();
	}

	void IncrementClearCounter() { Counter++; }

private:
	intrusive_ptr<oGPUDevice> Device;
	std::shared_ptr<window> ParentWindow;
	std::shared_ptr<window> GPUWindow;
	intrusive_ptr<oGPUCommandList> CommandList;
	intrusive_ptr<oGPURenderTarget> PrimaryRenderTarget;

	HWND hButton;
	int Counter;
	bool Running;
};

struct GPU_WindowInWindow : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus)
	{
		if (ouro::system::is_remote_session())
		{
			snprintf(_StrStatus, _SizeofStrStatus, "Detected remote session: differing text anti-aliasing will cause bad image compares");
			return SKIPPED;
		}

		// Turn display power on, otherwise the test will fail
		ouro::display::set_power_on();

		bool success = false;
		WindowInWindow test(&success);
		oTESTB0(success);

		if (kInteractiveMode)
		{
			while (test.IsRunning())
			{
				test.flush_messages();

				oStd::this_thread::sleep_for(oStd::chrono::seconds(1));
				test.IncrementClearCounter();

				test.Render();
			}
		}

		else
		{
			test.flush_messages();
			test.Render();
			oStd::future<std::shared_ptr<ouro::surface::buffer>> snapshot = test.GetWindow()->snapshot();
			while (!snapshot.is_ready()) { test.flush_messages(); }
			oTESTFI(snapshot);
			test.IncrementClearCounter();
			test.flush_messages();
			test.Render();
			snapshot = test.GetWindow()->snapshot();
			while (!snapshot.is_ready()) { test.flush_messages(); }
			oTESTFI2(snapshot, 1);
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(GPU_WindowInWindow);
