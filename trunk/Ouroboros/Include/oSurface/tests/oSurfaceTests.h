// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Declarations of oSurface unit tests. These throw on failure.
#pragma once
#ifndef oSurfaceTests_h
#define oSurfaceTests_h

namespace ouro {

	class test_services;

	namespace tests {

		void TESTsurface();
		void TESTsurface_bccodec(test_services& services);
		void TESTsurface_codec(test_services& services);
		void TESTsurface_fill(test_services& services);
		void TESTsurface_generate_mips(test_services& services);
		void TESTsurface_resize(test_services& services);

	} // namespace tests
} // namespace ouro

#endif
