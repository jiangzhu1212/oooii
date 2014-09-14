// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oSurface/codec.h>
#include <oSurface/fill.h>
#include <oBase/colors.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <oString/fixed_string.h>
#include <oCore/filesystem.h>

#include "../../test_services.h"

namespace ouro { 
	namespace tests {

static bool kSaveToDesktop = false;

void save_bmp_to_desktop(const surface::image& img, const char* _path)
{
	auto encoded = surface::encode(img
		, surface::file_format::bmp
		, default_allocator
		, default_allocator
		, surface::as_noax(img.get_info().format)
		, surface::compression::none);
	filesystem::save(filesystem::desktop_path() / path(_path), encoded, encoded.size());
}

static void compare_checkboards(const int2& dimensions, const surface::format& format, const surface::file_format& file_format, float max_rms)
{
	oTRACE("testing codec %s -> %s", as_string(format), as_string(file_format));

	// Create a buffer with a known format
	surface::info si;
	si.format = format;
	si.mip_layout = surface::mip_layout::none;
	si.dimensions = int3(dimensions, 1);
	surface::image known(si);
	size_t knownSize = known.size();
	{
		surface::lock_guard lock(known);
		surface::fill_checkerboard((color*)lock.mapped.data, lock.mapped.row_pitch, si.dimensions.xy(), si.dimensions.xy() / 2, blue, red);
	}

	size_t EncodedSize = 0;
	scoped_allocation encoded;

	{
		scoped_timer("encode");
		encoded = surface::encode(known
			, file_format
			, surface::as_noax(known.get_info().format)
			, surface::compression::none);
	}

	sstring buf;
	format_bytes(buf, EncodedSize, 2);
	oTRACE("encoded %s", buf.c_str());

	if (kSaveToDesktop)
	{
		mstring fname;
		snprintf(fname, "encoded_from_known_%s.%s", as_string(file_format), as_string(file_format));
		filesystem::save(path(fname), encoded, encoded.size());
	}

	surface::image decoded;
	{
		scoped_timer("decode");
		decoded = surface::decode(encoded, encoded.size(), format);
	}

	{
		size_t decodedSize = decoded.size();

		if (known.size() != decoded.size())
			oTHROW(io_error, "encoded %u but got %u on decode", knownSize, decodedSize);

		if (kSaveToDesktop)
		{
			mstring fname;
			snprintf(fname, "encoded_from_decoded_%s.%s", as_string(file_format), as_string(surface::file_format::bmp));
			save_bmp_to_desktop(decoded, fname);
		}

		float rms = surface::calc_rms(known, decoded);
		if (rms > max_rms)
			oTHROW(io_error, "encoded/decoded bytes mismatch for %s", as_string(file_format));
	}
}

void compare_load(test_services& services, const char* path, const char* desktop_filename_prefix)
{
	auto encoded = services.load_buffer(path);
	surface::image decoded;
	{
		scoped_timer("decode");
		decoded = surface::decode(encoded, encoded.size(), surface::format::b8g8r8a8_unorm);
	}

	auto ff = surface::get_file_format(path);

	mstring fname;
	snprintf(fname, "%s_%s.%s", desktop_filename_prefix, as_string(ff), as_string(surface::file_format::bmp));
	save_bmp_to_desktop(decoded, fname);
}

void TESTsurface_codec(test_services& _Services)
{
	// still a WIP
	//compare_checkboards(uint2(11,21), surface::format::b8g8r8a8_unorm, surface::file_format::psd, 1.0f);
	//compare_load(_Services, "Test/Textures/lena_1.psd", "lena_1");

	compare_checkboards(uint2(11,21), surface::format::b8g8r8a8_unorm, surface::file_format::bmp, 1.0f);
	compare_checkboards(uint2(11,21), surface::format::b8g8r8a8_unorm, surface::file_format::dds, 1.0f);
	compare_checkboards(uint2(11,21), surface::format::b8g8r8a8_unorm, surface::file_format::jpg, 4.0f);
	compare_checkboards(uint2(11,21), surface::format::b8g8r8a8_unorm, surface::file_format::png, 1.0f);
	compare_checkboards(uint2(11,21), surface::format::b8g8r8a8_unorm, surface::file_format::tga, 1.0f);
}

	}
}
