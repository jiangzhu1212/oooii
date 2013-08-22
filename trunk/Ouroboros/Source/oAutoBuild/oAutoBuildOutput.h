/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#pragma  once
#ifndef oAutoBuildOutput_h
#define oAutoBuildOutput_h

#include "oP4ChangelistBuilder.h"
#include "oMSBuild.h"
#include "oBuildTool.h"

struct oAutoBuildEmailSettings
{
	oNetAddr EmailServer;
	oStd::sstring FromAddress;
	oStd::sstring FromPasswordBase64;
	std::vector<oStd::sstring> AdminEmails;
	std::vector<oStd::sstring> UserEmails;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oAutoBuildEmailSettings)

struct oAutoBuildResults
{
	bool IsDailyBuild;
	int ChangeList;
	oStd::sstring BuildName;
	oStd::path_string OutputFolder;

	oMSBuildResults BuildResults;
	oUnitTestResults TestResults;
	oPackagingResults PackagingResults;
};

void oAutoBuildOutputResults(const oAutoBuildEmailSettings& _EmailSettings, int _WebServerPort, const oAutoBuildResults& _Results);
void oEmailAdminAndStop(const oAutoBuildEmailSettings& _EmailSettings, const char* _pMessage, int _CL, bool _HTML);

#endif