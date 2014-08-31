// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Declarations of oGPU unit tests. These throw on failure.
#pragma once
#ifndef oGPUTests_h
#define oGPUTests_h

namespace ouro {

	class test_services;

	namespace tests {

		void TESTbuffer();
		void TESTclear(test_services& _Services);
		void TESTdevice(test_services& _Services);
		void TESTinstanced_triangle(test_services& _Services);
		void TESTlines(test_services& _Services);
		void TESTquery();
		void TESTrender_target(test_services& _Services);
		void TESTspinning_triangle(test_services& _Services);
		void TESTtexture1d(test_services& _Services);
		void TESTtexture1dmip(test_services& _Services);
		void TESTtexture2d(test_services& _Services);
		void TESTtexture2dmip(test_services& _Services);
		void TESTtexture3d(test_services& _Services);
		void TESTtexture3dmip(test_services& _Services);
		void TESTtexturecube(test_services& _Services);
		void TESTtexturecubemip(test_services& _Services);
		void TESTtriangle(test_services& _Services);
		void TESTwindow_in_window(test_services& _Services);

	} // namespace tests
} // namespace ouro

#endif
