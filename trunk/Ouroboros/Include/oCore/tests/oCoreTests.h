// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Declarations of oCore unit tests. These throw on failure.
#pragma once
#ifndef oCoreTests_h
#define oCoreTests_h

namespace ouro {

	class test_services;

	namespace tests {

		void TESTadapter(test_services& _Services);
		void TESTcamera(test_services& _Services);
		void TESTcpu(test_services& _Services);
		void TESTdebugger(test_services& _Services);
		void TESTfilesystem(test_services& _Services);
		void TESTfilesystem_monitor();
		void TESTprocess_heap();
		#if defined(_WIN32) || defined(_WIN64)
			void TESTwin_crt_leak_tracker(test_services& _Services);
			void TESTwin_registry();
		#endif

	} // namespace tests
} // namespace ouro

#endif
