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
#include "oGPUTestCommon.h"
#include <oGPU/oGPUUtil.h>
#include <oCore/filesystem.h>

#include <oBasis/oMath.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

void gpu_test::create(const char* _Title, bool _DevMode, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size)
{
	NthSnapshot = 0;
	Running = true;
	DevMode = _DevMode;
	AllSnapshotsSucceeded = true;

	if (_pSnapshotFrameIDs && _NumSnapshotFrameIDs)
		SnapshotFrames.assign(_pSnapshotFrameIDs, _pSnapshotFrameIDs + _NumSnapshotFrameIDs);

	{
		device_init i;
		i.debug_name = "gpu_test.Device";
		i.version = version(10,0); // for broader compatibility
		i.driver_debug_level = debug_level::normal;
		Device = device::make(i);
	}

	{
		window::init i;
		i.title = _Title;
		i.alt_f4_closes = true;
		i.on_event = std::bind(&gpu_test::on_event, this, std::placeholders::_1);
		i.shape.state = DevMode ? window_state::restored : window_state::hidden;
		i.shape.style = window_style::sizable;
		i.shape.client_size = _Size;
		Window = window::make(i);
	}

	PrimaryRenderTarget = Device->make_primary_render_target(Window, surface::d24_unorm_s8_uint, true);
	PrimaryRenderTarget->set_clear_color(almost_black);

	CommandList = Device->get_immediate_command_list();
}

void gpu_test::on_event(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case event_type::closing:
			Running = false;
			break;
		default:
			break;
	}
}

void gpu_test::check_snapshot(test_services& _Services)
{
	const int FrameID = Device->frame_id();
	if (SnapshotFrames.end() != find(SnapshotFrames, FrameID))
	{
		std::shared_ptr<surface::buffer> snap = PrimaryRenderTarget->make_snapshot(0);
		_Services.check(snap, NthSnapshot);
		NthSnapshot++;
	}
}

void gpu_test::run(test_services& _Services)
{
	oASSERT(Window->is_window_thread(), "Run must be called from same thread that created the window");
	bool AllFramesSucceeded = true;

	// Flush window init
	Window->flush_messages();

	initialize();

	while (Running && (DevMode || Device->frame_id() < SnapshotFrames.back()))
	{
		Window->flush_messages();
		if (Device->begin_frame())
		{
			render();

			Device->end_frame();

			check_snapshot(_Services);

			if (DevMode)
				Device->present(1);
		}
		else
			oTRACEA("Frame %u failed", Device->frame_id());
	}

	if (!AllFramesSucceeded)
		oTHROW(protocol_error, "Image compares failed, see debug output/log for specifics.");
}

const int gpu_texture_test::sSnapshotFrames[2] = { 0, 2 };

void gpu_texture_test::initialize()
{
	TestConstants = Device->make_buffer<oGPUTestConstants>("TestConstants");
	Pipeline = Device->make_pipeline1(oGPUTestGetPipeline(get_pipeline()));
	Mesh = make_first_cube(Device);
	Texture = make_test_texture();
}

float gpu_texture_test::rotation_step()
{
	// this is -1 because there was a code change that resulted in begin_frame()
	// being moved out of the Render function below so it updated the frame ID
	// earlier than this code was ready for. If golden images are updated this
	// could go away.
	return (Device->frame_id()-1) * 1.0f;
}

void gpu_texture_test::render()
{
	float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -4.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

	render_target_info RTI = PrimaryRenderTarget->get_info();
	float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTI.dimensions.x / oCastAsFloat(RTI.dimensions.y), 0.001f, 1000.0f);

	float rotationStep = rotation_step();
	float4x4 W = make_rotation(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));

	CommandList->begin();

	commit_buffer(CommandList.get(), TestConstants.get(), oGPUTestConstants(W, V, P, white));

	CommandList->clear(PrimaryRenderTarget, clear_type::color_depth_stencil);
	CommandList->set_blend_state(blend_state::opaque);
	CommandList->set_depth_stencil_state(depth_stencil_state::test_and_write);
	CommandList->set_surface_state(surface_state::front_face);
	CommandList->set_buffer(0, TestConstants);
	CommandList->set_sampler(0, sampler_state::linear_wrap);
	CommandList->set_shader_resource(0, Texture);
	CommandList->set_pipeline(Pipeline);
	CommandList->set_render_target(PrimaryRenderTarget);
	Mesh->draw(CommandList);

	CommandList->end();
}

std::shared_ptr<surface::buffer> surface_load(const path& _Path, surface::alpha_option::value _Option)
{
	scoped_allocation b = filesystem::load(_Path);
	return surface::decode(b, b.size(), _Option);
}

std::shared_ptr<surface::buffer> make_1D(int _Width)
{
	surface::info si;
	si.dimensions = int3(_Width, 1, 1);
	si.format = surface::b8g8r8a8_unorm;
	auto s = surface::buffer::make(si);

	{
		surface::lock_guard lock(s);
		static const color sConsoleColors[] = { black, navy, green, teal, maroon, purple, olive, silver, gray, blue, lime, aqua, red, fuchsia, yellow, white };
		color* texture1Ddata = (color*)lock.mapped.data;
		for (int i = 0; i < si.dimensions.x; i++)
			texture1Ddata[i] = sConsoleColors[i % oCOUNTOF(sConsoleColors)];
	}

	return s;
}

	} // namespace tests
} // namespace ouro
