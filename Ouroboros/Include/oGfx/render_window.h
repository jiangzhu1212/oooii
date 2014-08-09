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
// Sets up a window on a separate thread and calls the main render loop.
#ifndef oGfx_render_window_h
#define oGfx_render_window_h

#include <oGPU/device.h>
#include <oGPU/primary_target.h>
#include <oGPU/depth_target.h>
#include <oGUI/window.h>
#include <thread>

namespace ouro { namespace gfx {

struct render_window_init
{
	render_window_init(const char* _name = "render_window")
		: name(_name)
		, icon(nullptr)
		, hotkeys(nullptr)
		, num_hotkeys(0)
		, color_format(surface::format::r8g8b8a8_unorm)
		, depth_format(surface::format::d24_unorm_s8_uint)
	{}

	const char* name;
	icon_handle icon;
	basic_hotkey_info* hotkeys;
	uint num_hotkeys;
	std::shared_ptr<window> parent;
	surface::format color_format;
	surface::format depth_format;

	input::action_hook on_action;
	std::function<void()> on_stop;
	std::function<void()> render;
};

class render_window
{
public:
	render_window() : win(nullptr), dev(nullptr), started(false) {}
	~render_window() { stop(); join(); }

	bool running() const { return started; }

	// starts a render_window thread that continuously calls the 
	// render() function and response to basic window events. This 
	// returns a pointer to the window whose lifetime is the same as
	// the thread. Do not retain a smart/refcounted reference to it
	// and protect usage of it any time after stop() is called.
	window* start(gpu::device& dev, const render_window_init& init);
	
	// this is asynchronous because the calling thread could be pumping
	// messages for the parent window which may need to be called to 
	// destroy this window. Ensure once all windows are destroyed join()
	// is called to ensure the thread is fully joined.
	void stop();

	inline bool joinable() const { return thread.joinable(); }
	inline void join() { thread.join(); }

	inline bool is_fullscreen_exclusive() const { return ctarget.is_fullscreen_exclusive(); }
	inline void set_fullscreen_exclusive(bool fullscreen) { return ctarget.set_fullscreen_exclusive(fullscreen); }

	inline gpu::primary_target& color_target() { return ctarget; }
	inline gpu::depth_target& depth_target() { return dtarget; }

private:
	std::shared_ptr<window> parent;
	gpu::primary_target ctarget;
	gpu::depth_target dtarget;
	window* win;
	gpu::device* dev;
	std::thread thread;
	bool started;
	render_window_init init;

	void run();
	std::shared_ptr<window> make_render_window(const render_window_init& init);
	void on_event(const window::basic_event& e);
};

}}

#endif
