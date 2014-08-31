// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGUI/tests/oGUITests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_GUI_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oGUI_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_GUI_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oGUI_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_GUI_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oGUI_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_GUI_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oGUI_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_GUI_TEST(WindowControls);
oTEST_REGISTER_GUI_TEST(SysDialog);
