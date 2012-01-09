// $(header)
#include <oBasis/oEightCC.h>
#include <oBasis/oStringize.h>
#include "oBasisTestCommon.h"
#include <cstring>

bool oBasisTest_oEightCC()
{
	oEightCC fcc('TEXT', 'URE ');

	char str[16];

	oTESTB(oToString(str, fcc), "oToString on oEightCC failed 1");
	oTESTB(!strcmp("TEXTURE ", str), "oToString on oEightCC failed 2");

	const char* fccStr = "GEOMETRY";
	oTESTB(oFromString(&fcc, fccStr), "oFromSTring on oEightCC failed 1");
	oTESTB(oEightCC('GEOM', 'ETRY') == fcc, "oFromSTring on oEightCC failed 2");

	return true;
}
