// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include <oSurface/surface.h>
#include <oSurface/codec.h>
#include <oCore/filesystem.h>

#include <oGfx/surface_view.h>

#include "../../test_services.h"

using namespace ouro::surface;

namespace ouro {
	namespace tests {

static void convert_and_test(test_services& services, gfx::surface_view& view, const format& target_format, const char* filename_suffix, uint nth_test, bool save_to_desktop = false)
{
	path TestImagePath = "Test/Textures/lena_1.png";

	auto file = services.load_buffer(TestImagePath);
	auto source = decode(file, file.size());
	
	oTRACEA("Converting image to %s...", as_string(target_format));
	auto converted_encoded = encode(source, file_format::dds, target_format);

	if (save_to_desktop)
	{
		path filename = TestImagePath.filename();
		filename.insert_basename_suffix(filename_suffix);
		filename.replace_extension(".dds");
		filesystem::save(filesystem::desktop_path() / filename, converted_encoded, converted_encoded.size());
	}

	// uncompress using the GPU since there's no code in ouro to decode BC formats
	auto decoded = decode(converted_encoded);
	view.set_texels(filename_suffix, decoded);
	view.render();
	auto snapshot = view.get_draw_target()->make_snapshot();
	services.check(snapshot, nth_test);
}

void TESTsurface_bccodec(test_services& services)
{
	gfx::core core("TESTsurface_bccodec", true);
	gpu::color_target ctarget;
	ctarget.initialize("TESTsurface_bccodec", core.device, surface::format::b8g8r8a8_unorm, 512, 512, 0, false);
	gfx::surface_view view(core);
	view.set_draw_target(ctarget);

	convert_and_test(services, view, format::bc1_unorm, "_BC1", 0);
	convert_and_test(services, view, format::bc3_unorm, "_BC3", 1);
	convert_and_test(services, view, format::bc7_unorm, "_BC7", 2);
	//convert_and_test(services, view, format::bc6h_uf16, "_BC6HU", 2);
}

	}
}
