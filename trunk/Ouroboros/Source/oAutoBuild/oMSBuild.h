// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma  once
#ifndef oMSBuild_h
#define oMSBuild_h
#include <oConcurrency/event.h>
#include <oBase/invalid.h>
#include <oBasis/oRTTI.h>
#include "oP4ChangelistBuilder.h"

struct oMSBUILD_SETTINGS
{
	oMSBUILD_SETTINGS()
		: TimeoutSeconds(0)
		, ToolPath("msbuild.exe")
		, CleanAlways(true)
		, ThreadsPerBuild(ouro::invalid)
	{}

	uint TimeoutSeconds;
	ouro::path_string ToolPath;
	ouro::path_string Solution;
	bool CleanAlways;
	std::vector<ouro::sstring> Configurations;
	std::vector<ouro::sstring> Platforms;
	int ThreadsPerBuild;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oMSBUILD_SETTINGS)

struct oMSBuildResults
{
	bool CleanSucceeded;
	bool BuildSucceeded;
	bool SavingLogfilesSucceeded;

	float CleanTimeSeconds;
	float BuildTimeSeconds;
	float SavingLogfilesTimeSeconds;

	bool BuildTimedOut;
	std::vector<ouro::path_string> BuildLogfiles;
};

typedef ouro::xxlstring o_msbuild_stdout_t;

// Returns std::errc::protocol_error if the build fails due to a bad build.  
// Returns std::errc::invalid_argument if the build tool can't run.
bool oMSBuildAndLog(const oMSBUILD_SETTINGS& _Settings, const char* _pLogFolder, const ouro::event& _CancelEvent, oMSBuildResults* _pResults);

// Returns false if the log file cannot be opened or read
bool oMSBuildParseLogfile(ouro::path_string _Logfile, bool _IncludeWarnings, std::function<bool(o_msbuild_stdout_t _WarningOrError)> _Output);

#endif