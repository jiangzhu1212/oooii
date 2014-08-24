/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#pragma once
#ifndef oGPUTestCommon_h
#define oGPUTestCommon_h

#include <oGPU/all.h>
#include "oGPU/oGPUUtilMesh.h"
#include "oGPUTestHLSL.h"

#include <oGUI/window.h>

#include <oGPU/shaders.h>

#include "../../test_services.h"

#define oGPU_COMMON_TEST(_Name) void TEST##_Name(test_services& _Services) { gpu_test_##_Name t; t.run(_Services); }

namespace ouro {
	namespace tests {

class gpu_test
{
public:
	struct pipeline
	{
		pipeline()
			: input(gpu::intrinsic::vertex_layout::pos)
			, vs(gpu::intrinsic::vertex_shader::pass_through_pos)
			, ps(gpu::intrinsic::pixel_shader::white)
		{}

		gpu::intrinsic::vertex_layout::value input;
		gpu::intrinsic::vertex_shader::value vs;
		gpu::intrinsic::pixel_shader::value ps;
	};

	gpu_test(const char* _Title, bool _Interactive, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size = int2(640, 480))
		: FrameID(0)
	{
		create(_Title, _Interactive, _pSnapshotFrameIDs, _NumSnapshotFrameIDs, _Size);
	}
	
	template<size_t size>
	gpu_test(const char* _Title, bool _Interactive, const int (&_pSnapshotFrameIDs)[size], const int2& _Size = int2(640, 480))
		: FrameID(0)
	{
		create(_Title, _Interactive, _pSnapshotFrameIDs, size, _Size);
	}

	virtual ~gpu_test() {}

	// Called once before rendering begins (throw on error)
	// the override must return
	virtual pipeline initialize() { return pipeline(); }

	// Infrastructure calls begin_frame/end_frame; the rest is up to this function
	virtual void render() = 0;

	void run(test_services& _Services);

	window* get_window() { return Window.get(); }
	gpu::device& device() { return Device; }
	gpu::command_list& get_command_list() { return Device.immediate(); }

	color get_clear_color() const { return almost_black; }

protected:
	bool is_devmode() const { return DevMode; }

private:
	void on_event(const window::basic_event& _Event);
	void create(const char* _Title, bool _Interactive, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size);

	// Call this after each device::end_frame() and before device::present to 
	// check if the current frame has been registered for testing. If so do the
	// test and oTRACEA out any failure and flag ultimate failure for later. If an
	// early-out is desired respond to a false return value, otherwise this will 
	// have run return false when it is finished.
	void check_snapshot(test_services& _Services);

	std::vector<int> SnapshotFrames;

	int NthSnapshot;
	bool Running;
	bool DevMode;
	bool AllSnapshotsSucceeded;

protected:
	std::shared_ptr<window> Window;
	gpu::device Device;
	gpu::blend_state BlendState;
	gpu::depth_stencil_state DepthStencilState;
	gpu::rasterizer_state RasterizerState;
	gpu::sampler_state SamplerState;
	gpu::vertex_layout VertexLayout;
	gpu::vertex_shader VertexShader;
	gpu::pixel_shader PixelShader;
	gpu::primary_target PrimaryColorTarget;
	gpu::depth_target PrimaryDepthTarget;
	gpu::constant_buffer TestConstants;
	int FrameID;
};

class gpu_texture_test : public gpu_test
{
public:
	gpu_texture_test(const char* _Title, bool _Interactive, const int2& _Size = int2(640, 480))
		: gpu_test(_Title, _Interactive, sSnapshotFrames, oCOUNTOF(sSnapshotFrames), _Size)
	{}

	virtual pipeline get_pipeline() = 0;
	virtual gpu::resource* make_test_texture() = 0;
	virtual float rotation_step();

	pipeline initialize() override;
	void render() override;

protected:
	gpu::resource* Resource;
	gpu::util_mesh Mesh;

	static const int sSnapshotFrames[2];
};

surface::image surface_load(const path& _Path, bool _Mips);

surface::image make_1D(int _Width, bool _Mips);

	} // namespace tests
} // namespace ouro

#endif
