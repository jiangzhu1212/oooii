// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include <oCore/system.h>
#include <oBasis/tests/oBasisTests.h>
#include "oTestIntegration.h"

using namespace ouro;

// Tests from oBasis follow a common form, so as a convenience and 
// centralization, use this macro to get through the boilerplate bridging from
// that test call to oTest's infrastructure.

#define oTESTB0_BASIS(expr) do \
	{	RESULT r = oTest::SUCCESS; \
		if (!(expr)) \
		{	switch (oErrorGetLast()) \
			{	case std::errc::permission_denied: r = oTest::SKIPPED; break; \
				case 0: r = oTest::SUCCESS; break; \
				default: r = oTest::FAILURE; break; \
			} \
		} \
		snprintf(_StrStatus, _SizeofStrStatus, "%s", oErrorGetLastString()); \
		oTRACE("%s: %s (oErrorGetLast() == %s)", ouro::as_string(r), _StrStatus, oErrorAsString(oErrorGetLast())); \
		return r; \
	} while (false)

#define oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(oBasis_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oTESTB0_BASIS(oCONCAT(oBasisTest_, _BasisTestName)()); \
		} \
	};

#define oTEST_REGISTER_BASIS_TEST(_BasisTestName) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER(oCONCAT(oBasis_, _BasisTestName))

#define oTEST_REGISTER_BASIS_TEST_BUGGED(_BasisTestName, _Bug) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER_BUGGED(oCONCAT(oBasis_, _BasisTestName), _Bug)

#define oTEST_REGISTER_BASIS_TEST_BUGGED32(_BasisTestName, _Bug) \
	oTEST_WRAP_BASIS_TEST(_BasisTestName) \
	oTEST_REGISTER_BUGGED32(oCONCAT(oBasis_, _BasisTestName), _Bug)

#define oTEST_REGISTER_BASIS_TEST_WITH_SERVICES(_BasisTestName) \
	bool oCONCAT(oBasisTest_, _BasisTestName)(); \
	struct oCONCAT(oBasis_, _BasisTestName) : oTest \
	{	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override \
		{	oBasisTestServices Services; \
			oInitBasisServices(this, &Services); \
			oTESTB0_BASIS(oCONCAT(oBasisTest_, _BasisTestName)(Services)); \
		} \
	}; \
	oTEST_REGISTER(oCONCAT(oBasis_, _BasisTestName))

oTEST_REGISTER_BASIS_TEST(oINISerialize);
oTEST_REGISTER_BASIS_TEST(oJSONSerialize);
oTEST_REGISTER_BASIS_TEST(oMath);
oTEST_REGISTER_BASIS_TEST(oRTTI);
oTEST_REGISTER_BASIS_TEST(oString);
oTEST_REGISTER_BASIS_TEST(oURI);
oTEST_REGISTER_BASIS_TEST(oURIQuerySerialize);
oTEST_REGISTER_BASIS_TEST(oXMLSerialize);
