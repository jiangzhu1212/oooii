// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma  once
#ifndef oBuildTool_h
#define oBuildTool_h
#include <oBase/event.h>

struct oBUILD_TOOL_TESTING_SETTINGS
{
	bool ReSync;
	uint TimeoutSeconds;
	ouro::path_string CommandLine;
	ouro::path_string FailedImageCompares;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oBUILD_TOOL_TESTING_SETTINGS)

struct oBUILD_TOOL_PACKAGING_SETTINGS
{
	uint TimeoutSeconds;
	std::vector<ouro::path_string> CommandLines;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oBUILD_TOOL_PACKAGING_SETTINGS)

struct oUnitTestResults
{
	float TimePassedSeconds;
	bool HasTimedOut;
	bool ParseLogfileSucceeded;
	bool TestingSucceeded;
	ouro::uri_string StdoutLogfile;
	ouro::uri_string StderrLogfile;
	ouro::uri_string FailedImagePath;

	struct TestItem
	{
		ouro::sstring Name;
		ouro::sstring Status;
		ouro::lstring Message;
	};
	std::vector<TestItem> FailedTests;
};

struct oPackagingResults
{
	float PackagingTimeSeconds;
};

bool oRunTestingStage(const oBUILD_TOOL_TESTING_SETTINGS& _TestSettings, const char* _BuildRoot, const ouro::event& _CancelEvent, oUnitTestResults* _pResults);
bool oRunPackagingStage(const oBUILD_TOOL_PACKAGING_SETTINGS& _Settings, oPackagingResults* _pResults);


#endif //oBuildTool_h