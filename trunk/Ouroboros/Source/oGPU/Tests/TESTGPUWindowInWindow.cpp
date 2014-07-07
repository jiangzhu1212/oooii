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
#include <oGPU/oGPU.h>
#include <oGPU/primary_target.h>
#include <oGUI/window.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/display.h>
#include <oCore/system.h>

#include "../../test_services.h"

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const bool kInteractiveMode = false;

class WindowInWindow
{
public:
	WindowInWindow()
		: Counter(0)
		, Running(true)
	{
		// Create the parent
		{
			window::init i;
			i.title = "Window-In-Window Test";
			i.on_event = std::bind(&WindowInWindow::ParentEventHook, this, std::placeholders::_1);
			i.shape.state = window_state::hidden;
			i.shape.style = window_style::sizable;
			i.shape.client_size = int2(640, 480);
			ParentWindow = window::make(i);
		}

		// Create the device
		{
			device_init i;
			i.debug_name = "TestDevice";
			i.version = version(10,0);
			i.driver_debug_level = gpu::debug_level::normal;
			Device = device::make(i);
		}

		{
			command_list_info i;
			i.draw_order = 0;
			CommandList = Device->make_command_list("CL0", i);
		}

		{
			window::init i;
			i.shape.state = window_state::restored;
			i.shape.style = window_style::borderless;
			i.shape.client_position = int2(20,20);
			i.shape.client_size = int2(600,480-65);
			i.on_event = std::bind(&WindowInWindow::GPUWindowEventHook, this, std::placeholders::_1);

			GPUWindow = window::make(i);
			PrimaryTarget.initialize(GPUWindow.get(), Device.get(), true);
			GPUWindow->parent(ParentWindow);
		}

		ParentWindow->show();
	}

	inline bool running() const { return Running; }

	void render()
	{
		if (!PrimaryTarget)
			return;

		if (Device->begin_frame())
		{
			CommandList->begin();
			PrimaryTarget.set_draw_target(CommandList.get());
			PrimaryTarget.clear(CommandList.get(), (Counter & 0x1) ? white : blue);
			CommandList->end();

			Device->end_frame();
			PrimaryTarget.present();
		}
	}

	void ParentEventHook(const window::basic_event& _Event)
	{
		switch (_Event.type)
		{
			case event_type::creating:
			{
				control_info ButtonDesc;
				ButtonDesc.parent = _Event.window;
				ButtonDesc.type = control_type::button;
				ButtonDesc.text = "Push Me";
				ButtonDesc.size = int2(100,25);
				ButtonDesc.position = int2(10,480-10-ButtonDesc.size.y);
				ButtonDesc.id = 0;
				ButtonDesc.starts_new_group = false;
				hButton = oWinControlCreate(ButtonDesc);
				break;
			}

			case event_type::closing:
				Running = false;
				break;

			case event_type::sized:
			{
				if (GPUWindow)
					GPUWindow->client_size(_Event.as_shape().shape.client_size - int2(40,65));

				if (PrimaryTarget)
					PrimaryTarget.resize(int2(_Event.as_shape().shape.client_size - int2(40,65)));

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

	window* get_window() { return ParentWindow.get(); }

	std::shared_ptr<surface::buffer> snapshot_and_wait()
	{
		future<std::shared_ptr<surface::buffer>> snapshot = ParentWindow->snapshot();
		while (!snapshot.is_ready()) { flush_messages(); }
		return snapshot.get();
	}

	void flush_messages()
	{
		GPUWindow->flush_messages();
		ParentWindow->flush_messages();
	}

	void increment_clear_counter() { Counter++; }

private:
	std::shared_ptr<device> Device;
	std::shared_ptr<window> ParentWindow;
	std::shared_ptr<window> GPUWindow;
	std::shared_ptr<command_list> CommandList;
	primary_target PrimaryTarget;

	HWND hButton;
	int Counter;
	bool Running;
};

void TESTwindow_in_window(test_services& _Services)
{
	if (_Services.is_remote_session())
	{
		_Services.report("Detected remote session: differing text anti-aliasing will cause bad image compares");
		oTHROW(not_supported, "Detected remote session: differing text anti-aliasing will cause bad image compares");
	}

	// Turn display power on, otherwise the test will fail
	display::set_power_on();

	bool success = false;
	WindowInWindow test;

	if (kInteractiveMode)
	{
		while (test.running())
		{
			test.flush_messages();

			std::this_thread::sleep_for(std::chrono::seconds(1));
			test.increment_clear_counter();

			test.render();
		}
	}

	else
	{
		test.flush_messages();
		test.render();
		std::shared_ptr<surface::buffer> snapshot = test.snapshot_and_wait();
		_Services.check(snapshot);
		test.increment_clear_counter();
		test.flush_messages();
		test.render();
		snapshot = test.snapshot_and_wait();
		_Services.check(snapshot, 1);
	}
}

	} // namespace tests
} // namespace ouro
