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
#ifndef oMSBuild_h
#define oMSBuild_h
#include <oConcurrency/event.h>
#include "oP4ChangelistBuilder.h"

struct oMSBUILD_SETTINGS
{
	oMSBUILD_SETTINGS()
		: TimeoutSeconds(0)
		, ToolPath("msbuild.exe")
		, CleanAlways(true)
		, ThreadsPerBuild(oInvalid)
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
bool oMSBuildAndLog(const oMSBUILD_SETTINGS& _Settings, const char* _pLogFolder, const oConcurrency::event& _CancelEvent, oMSBuildResults* _pResults);

// Returns false if the log file cannot be opened or read
bool oMSBuildParseLogfile(ouro::path_string _Logfile, bool _IncludeWarnings, oFUNCTION<bool(o_msbuild_stdout_t _WarningOrError)> _Output);

#endif