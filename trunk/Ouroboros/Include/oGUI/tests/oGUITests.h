// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Declarations of oGUI unit tests. These throw on failure.
#pragma once
#ifndef oGUITests_h
#define oGUITests_h

namespace ouro {

	class test_services;

	namespace tests {

		void TESTWindowControls(test_services& _Services);
		void TESTSysDialog(test_services& _Services);

	}
}

#endif
