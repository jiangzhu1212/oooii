/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#ifndef oBuildTool_h
#define oBuildTool_h
#include <oConcurrency/event.h>

struct oBUILD_TOOL_TESTING_SETTINGS
{
	bool ReSync;
	uint TimeoutSeconds;
	oStd::path_string CommandLine;
	oStd::path_string FailedImageCompares;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oBUILD_TOOL_TESTING_SETTINGS)

struct oBUILD_TOOL_PACKAGING_SETTINGS
{
	uint TimeoutSeconds;
	std::vector<oStd::path_string> CommandLines;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oBUILD_TOOL_PACKAGING_SETTINGS)

struct oUnitTestResults
{
	float TimePassedSeconds;
	bool HasTimedOut;
	bool ParseLogfileSucceeded;
	bool TestingSucceeded;
	oStd::uri_string StdoutLogfile;
	oStd::uri_string StderrLogfile;
	oStd::uri_string FailedImagePath;

	struct TestItem
	{
		oStd::sstring Name;
		oStd::sstring Status;
		oStd::lstring Message;
	};
	std::vector<TestItem> FailedTests;
};

struct oPackagingResults
{
	float PackagingTimeSeconds;
};

bool oRunTestingStage(const oBUILD_TOOL_TESTING_SETTINGS& _TestSettings, const char* _BuildRoot, const oConcurrency::event& _CancelEvent, oUnitTestResults* _pResults);
bool oRunPackagingStage(const oBUILD_TOOL_PACKAGING_SETTINGS& _Settings, oPackagingResults* _pResults);


#endif //oBuildTool_h