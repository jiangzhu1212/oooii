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
#pragma once
#ifndef oGPUTestCommon_h
#define oGPUTestCommon_h

#include <oGPU/oGPUUtilMesh.h>
#include "oGPUTestPipelines.h"
#include "oGPUTestHLSL.h"

#include <oGPU/oGPU.h>
#include <oGUI/window.h>

#include "../../test_services.h"

#define oGPU_COMMON_TEST(_Name) void TEST##_Name(test_services& _Services) { gpu_test_##_Name t; t.run(_Services); }

namespace ouro {
	namespace tests {

class gpu_test
{
public:
	gpu_test(const char* _Title, bool _Interactive, const int* _pSnapshotFrameIDs, size_t _NumSnapshotFrameIDs, const int2& _Size = int2(640, 480))
	{
		create(_Title, _Interactive, _pSnapshotFrameIDs, _NumSnapshotFrameIDs, _Size);
	}
	
	template<size_t size>
	gpu_test(const char* _Title, bool _Interactive, const int (&_pSnapshotFrameIDs)[size], const int2& _Size = int2(640, 480))
	{
		create(_Title, _Interactive, _pSnapshotFrameIDs, size, _Size);
	}

	virtual ~gpu_test() {}

	// Called once before rendering begins (throw on error)
	virtual void initialize() {}

	// Infrastructure calls begin_frame/end_frame; the rest is up to this function
	virtual void render() = 0;

	void run(test_services& _Services);

	window* get_window() { return Window.get(); }
	gpu::device* get_device() { return Device.get(); }
	gpu::command_list* get_command_list() { return CommandList.get(); }
	gpu::render_target* get_primary_render_target() { return PrimaryRenderTarget.get(); }

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
	std::shared_ptr<gpu::device> Device;
	std::shared_ptr<gpu::command_list> CommandList;
	std::shared_ptr<gpu::render_target> PrimaryRenderTarget;
};

class gpu_texture_test : public gpu_test
{
public:
	gpu_texture_test(const char* _Title, bool _Interactive, const int2& _Size = int2(640, 480))
		: gpu_test(_Title, _Interactive, sSnapshotFrames, oCOUNTOF(sSnapshotFrames), _Size)
	{}

	virtual oGPU_TEST_PIPELINE get_pipeline() = 0;
	virtual std::shared_ptr<gpu::texture> make_test_texture() = 0;
	virtual float rotation_step();

	void initialize() override;
	void render() override;

protected:
	std::shared_ptr<gpu::pipeline1> Pipeline;
	std::shared_ptr<gpu::texture> Texture;
	gpu::util_mesh Mesh;
	std::shared_ptr<gpu::buffer> TestConstants;

	static const int sSnapshotFrames[2];
};

std::shared_ptr<surface::buffer> surface_load(const path& _Path, surface::alpha_option::value _Option = surface::alpha_option::force_alpha);

std::shared_ptr<surface::buffer> make_1D(int _Width);

	} // namespace tests
} // namespace ouro

#endif
