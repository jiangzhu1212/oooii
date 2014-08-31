// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oMesh/tests/oMeshTests.h>
#include "oTestIntegration.h"

using namespace ouro::tests;

#define oTEST_REGISTER_MESH_TEST0(_Name) oTEST_THROWS_REGISTER0(oCONCAT(oMesh_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_MESH_TEST(_Name) oTEST_THROWS_REGISTER(oCONCAT(oMesh_, _Name), oCONCAT(TEST, _Name))

#define oTEST_REGISTER_MESH_TEST_BUGGED0(_Name) oTEST_THROWS_REGISTER_BUGGED0(oCONCAT(oMesh_, _Name), oCONCAT(TEST, _Name))
#define oTEST_REGISTER_MESH_TEST_BUGGED(_Name) oTEST_THROWS_REGISTER_BUGGED(oCONCAT(oMesh_, _Name), oCONCAT(TEST, _Name))

oTEST_REGISTER_MESH_TEST(obj);
