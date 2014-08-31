// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef Ouroboros_Externals_h
#define Ouroboros_Externals_h

// Describes summary and description strings for each of the 3rd-party open-
// source software used by Ouroboros.

// To use:
// #define oOUROBOROS_EXTERNAL(_StrName, _StrVersion, _UrlHome, _UrlLicense, _StrDesc) <your-impl-here>
#define oOUROBOROS_EXTERNALS \
	oOUROBOROS_EXTERNAL("Ouroboros", "", "http://code.google.com/p/oooii/", "http://opensource.org/licenses/mit-license.php", "") \
	oOUROBOROS_EXTERNAL("bullet", "v2.82", "http://bulletphysics.org/wordpress/", "https://code.google.com/p/bullet/source/browse/tags/bullet-2.82/BulletLicense.txt", "vectormath lib only.") \
	oOUROBOROS_EXTERNAL("calfaq", "", "http://www.tondering.dk/claus/calendar.html", "http://www.boost.org/LICENSE_1_0.txt", "") \
	oOUROBOROS_EXTERNAL("ispc_texcomp", "r2", "https://software.intel.com/en-us/articles/fast-ispc-texture-compressor-update", "https://software.intel.com/en-us/articles/fast-ispc-texture-compressor-update", "") \
	oOUROBOROS_EXTERNAL("libjpegTurbo", "Version 8b", "http://libpng.sourceforge.net/index.html", "http://sourceforge.net/p/libjpeg-turbo/code/HEAD/tree/trunk/README-turbo.txt", "") \
	oOUROBOROS_EXTERNAL("libpng", "v1.6.6", "http://libpng.sourceforge.net/index.html", "http://sourceforge.net/p/libpng/code/ci/master/tree/LICENSE", "") \
	oOUROBOROS_EXTERNAL("lzma", "v9.20", "http://www.7-zip.org/sdk.html", "http://www.7-zip.org/sdk.html", "") \
	oOUROBOROS_EXTERNAL("OpenEXR", "v1.0.3", "http://www.openexr.com/", "http://www.openexr.com/license.html", "") \
	oOUROBOROS_EXTERNAL("smhasher", "R150", "http://code.google.com/p/smhasher/", "http://opensource.org/licenses/mit-license.php", "") \
	oOUROBOROS_EXTERNAL("snappy", "v1.0.3", "http://code.google.com/p/snappy/", "http://code.google.com/p/snappy/source/browse/trunk/COPYING", "") \
	oOUROBOROS_EXTERNAL("tbb", "v4.2 update 2", "https://www.threadingbuildingblocks.org/", "https://www.threadingbuildingblocks.org/licensing", "") \
	oOUROBOROS_EXTERNAL("tlsf", "v3.0", "http://tlsf.baisoku.org/", "http://tlsf.baisoku.org/", "") \
	oOUROBOROS_EXTERNAL("zlib", "v1.2.5", "http://www.zlib.net/", "https://www.threadingbuildingblocks.org/licensing", "")

#endif
