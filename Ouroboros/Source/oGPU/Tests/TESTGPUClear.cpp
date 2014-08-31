// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"

#include <oGUI/Windows/oWinRect.h>

#include <oGUI/window.h>

using namespace ouro::gpu;

namespace ouro {
	namespace tests {

static const int sSnapshotFrames[] = { 0, 1 };
static const bool kIsDevMode = false;

class gpu_test_clear : public gpu_test
{
public:
	gpu_test_clear() : gpu_test("GPU test: clear", kIsDevMode, sSnapshotFrames) {}

	void render() override
	{
		static color sClearColors[] = { lime, white };
		static int FrameID = 0;
		PrimaryColorTarget.clear(get_command_list(), sClearColors[FrameID++ % oCOUNTOF(sClearColors)]);
	}
};

oGPU_COMMON_TEST(clear);

	} // namespace tests
} // namespace ouro
