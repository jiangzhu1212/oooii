// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGfx/render_window.h>
#include <oBase/backoff.h>
#include <oCore/thread_traits.h>
#include <oGUI/msgbox.h>

namespace ouro { namespace gfx {

window* render_window::start(gpu::device& _dev, const render_window_init& _init)
{
	oCHECK(!running(), "render_window already started");
	dev = &_dev;
	init = _init;
	started = true;
	thread = std::thread(&render_window::run, this);
	backoff bo;
	while (!win)
		bo.pause();
	return win;
}

void render_window::stop()
{
	started = false;
}

void render_window::on_event(const window::basic_event& e)
{
	switch (e.type)
	{
		case event_type::sized:
		{
			if (ctarget)
			{
				// targets deinit on 0 dimensions, so protect against that
				int2 NewSize = max(int2(1,1), e.as_shape().shape.client_size);
				ctarget.resize(NewSize);
				if (dtarget)
					dtarget.resize(NewSize);
			}
			break;
		}

		case event_type::closing:
			started = false;
			break;

	default:
		break;
	}
}

std::shared_ptr<window> render_window::make_render_window(const render_window_init& init)
{
	window::init i;
	i.title = init.name;
	i.icon = init.icon;
	i.on_action = init.on_action;
	i.on_event = std::bind(&render_window::on_event, this, std::placeholders::_1);

	// set up in an initial small position, hidden until all parenting is set up
	const uint2 init_size(1, 1);
	i.shape.client_position = int2(0, 0);
	i.shape.client_size = init_size;
	i.shape.state = window_state::hidden;
	i.shape.style = window_style::borderless;
	i.cursor_state = cursor_state::hand;
	i.alt_f4_closes = true;
	auto w = window::make(i);

	ctarget.initialize(w.get(), *dev, true);
	if (init.depth_format != surface::format::unknown)
	{
		sstring n;
		snprintf(n, "%s depth", init.name ? init.name : "primary");
		dtarget.initialize(n, *dev, init.depth_format, init_size.x, init_size.y, 0, false, 0);
	}
	
	// set up basic event handling parenting and set to show (won't be visible unless parent is)
	w->set_hotkeys(init.hotkeys, init.num_hotkeys);
	w->parent(init.parent);
	w->show();
	return w;
}

void render_window::run()
{
	core_thread_traits::begin_thread(init.name ? init.name : "render_window");
	{
		std::shared_ptr<window> gpu_window;
		{
			gpu_window = make_render_window(init);
			win = gpu_window.get();
		}

		try
		{
			while (started)
			{
				win->flush_messages();
				if (init.render)
					init.render();
			}
		}

		catch (std::exception& e)
		{
			msgbox(msg_type::info, nullptr, "render_window", "ERROR\n%s", e.what());
		}

		// clean up state most
		ctarget.deinitialize();
		dtarget.deinitialize();
		dev = nullptr;
		started = false;
		parent = nullptr;
		win = nullptr;
	}

	if (init.on_stop)
		init.on_stop();

	init = render_window_init();
	core_thread_traits::end_thread();
}

}}
