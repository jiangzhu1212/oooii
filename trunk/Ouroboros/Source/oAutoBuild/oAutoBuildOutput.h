// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma  once
#ifndef oAutoBuildOutput_h
#define oAutoBuildOutput_h

#include "oP4ChangelistBuilder.h"
#include "oMSBuild.h"
#include "oBuildTool.h"
#include <oPlatform/oSocket.h>

struct oAutoBuildEmailSettings
{
	oNetAddr EmailServer;
	ouro::sstring FromAddress;
	ouro::sstring FromPasswordBase64;
	std::vector<ouro::sstring> AdminEmails;
	std::vector<ouro::sstring> UserEmails;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oAutoBuildEmailSettings)

struct oAutoBuildResults
{
	bool IsDailyBuild;
	int ChangeList;
	ouro::sstring BuildName;
	ouro::path_string OutputFolder;

	oMSBuildResults BuildResults;
	oUnitTestResults TestResults;
	oPackagingResults PackagingResults;
};

void oAutoBuildOutputResults(const oAutoBuildEmailSettings& _EmailSettings, int _WebServerPort, const oAutoBuildResults& _Results);
void oEmailAdminAndStop(const oAutoBuildEmailSettings& _EmailSettings, const char* _pMessage, int _CL, bool _HTML);

#endif