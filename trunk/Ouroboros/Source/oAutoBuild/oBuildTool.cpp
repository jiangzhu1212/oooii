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
#include "oBuildTool.h"

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oBUILD_TOOL_TESTING_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oBUILD_TOOL_TESTING_SETTINGS)
	oRTTI_COMPOUND_VERSION(oBUILD_TOOL_TESTING_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oBUILD_TOOL_TESTING_SETTINGS)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, ReSync, oRTTI_OF(bool), "ReSync", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, TimeoutSeconds, oRTTI_OF(uint), "TimeoutSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, CommandLine, oRTTI_OF(ostd_path_string), "CommandLine", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, FailedImageCompares, oRTTI_OF(ostd_path_string), "FailedImageCompares", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oBUILD_TOOL_TESTING_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oBUILD_TOOL_TESTING_SETTINGS)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oBUILD_TOOL_PACKAGING_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oBUILD_TOOL_PACKAGING_SETTINGS)
	oRTTI_COMPOUND_VERSION(oBUILD_TOOL_PACKAGING_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oBUILD_TOOL_PACKAGING_SETTINGS)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_PACKAGING_SETTINGS, TimeoutSeconds, oRTTI_OF(uint), "TimeoutSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_PACKAGING_SETTINGS, CommandLines, oRTTI_OF(std_vector_ostd_path_string), "CommandLines", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oBUILD_TOOL_PACKAGING_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oBUILD_TOOL_PACKAGING_SETTINGS)

static std::regex TestErrorParse("(.+?)\\s*³\\s*(FAILURE|LEAKS)\\s*³(?:.+?)³\\s*(.+?)\\n", std::tr1::regex_constants::optimize); // @oooii-kevin: ok static (duplication won't affect correctness)

bool oRunTestingStage(const oBUILD_TOOL_TESTING_SETTINGS& _TestSettings, const char* _BuildRoot, const oConcurrency::event& _CancelEvent, oUnitTestResults* _pResults)
{
	_pResults->TimePassedSeconds = 0.0f;

	// Clean the failed image directory
	oStreamDelete(_TestSettings.FailedImageCompares);

	oStd::path_string command_line = _TestSettings.CommandLine;
	oStrAppendf(command_line, " -z -l %soUnitTests.txt", _BuildRoot);

	// oUnitTests adds stdout/stderr to the filename
	oPrintf(_pResults->StdoutLogfile, "%soUnitTests.stdout.txt", _BuildRoot);
	oPrintf(_pResults->StderrLogfile, "%soUnitTests.stderr.txt", _BuildRoot);

	oProcess::DESC TestDesc;
	TestDesc.CommandLine = command_line.c_str();
	TestDesc.StartSuspended = false;

	oRef<threadsafe oProcess> TestProcess;
	if (!oProcessCreate(TestDesc, &TestProcess))
		return oErrorSetLast(std::errc::invalid_argument);

	uint TimeoutMS = _TestSettings.TimeoutSeconds * 1000;
	oScopedPartialTimeout Timer(&TimeoutMS);
	bool Finished = false;
	while(!Finished && TimeoutMS > 0)
	{
		if (_CancelEvent.is_set())
		{
			TestProcess->Kill(0);
			return oErrorSetLast(std::errc::operation_canceled);
		}

		Finished = TestProcess->Wait(500);
		Timer.UpdateTimeout();
	}
	// Always kill the process 
	TestProcess->Kill(0);

	// If timed out, wait a little bit otherwise the log files will get a
	// sharing violation when we try to read them.
	if (!Finished)
		oSleep(3000);

	_pResults->TimePassedSeconds = (float)((1000*_TestSettings.TimeoutSeconds) - TimeoutMS) / 1000.0f;

	// Copy failed images to the build root
	oStd::uri_string FailedImagesSource;
	oURIFromAbsolutePath(FailedImagesSource, _TestSettings.FailedImageCompares);
	oStd::uri_string FailedImagesDestination;
	oURIFromAbsolutePath(FailedImagesDestination, _BuildRoot);
	oStreamCopy(FailedImagesSource, FailedImagesDestination);
	_pResults->FailedImagePath = _BuildRoot;

	// Parse log file
	_pResults->ParseLogfileSucceeded = true;
	oRef<oBuffer> TestResultsBuffer;
	if (!oBufferLoad(_pResults->StdoutLogfile, &TestResultsBuffer, true))
	{
		_pResults->ParseLogfileSucceeded = false;
		oErrorPrefixLast("Failed to load log file %s because ", _pResults->StdoutLogfile.c_str());
		return false;
	}

	std::string TestResults = (const char*)TestResultsBuffer->GetData();

	const std::regex_token_iterator<std::string::iterator> end;
	int arr[] = {1, 2, 3}; 

	for(std::regex_token_iterator<std::string::iterator> VecTok(TestResults.begin(), TestResults.end(), TestErrorParse, arr); VecTok != end; ++VecTok)
	{
		oUnitTestResults::TestItem item;
		item.Name = VecTok->str().c_str(); ++VecTok;
		item.Status = VecTok->str().c_str(); ++VecTok;
		item.Message = VecTok->str().c_str();
		_pResults->FailedTests.push_back(item);
	}

	_pResults->HasTimedOut = !Finished;
	_pResults->TestingSucceeded = Finished && _pResults->FailedTests.size() == 0;
	return true;
}

bool oRunPackagingStage(const oBUILD_TOOL_PACKAGING_SETTINGS& _Settings, oPackagingResults* _pResults)
{
	float PackageStart = oTimerMSF();
	if (!_Settings.CommandLines.empty())
	{
		// FIXME: With CL 23502 the process will hang because of /S on Xcopy. Not capturing STDOUT fixes it
		// oStd::xxlstring PackageResponse;
		int ExitCode = 0;
		oFOR(auto& command_line, _Settings.CommandLines)
		{
			if (!oSystemExecute(command_line, nullptr, 0, &ExitCode, _Settings.TimeoutSeconds * 1000))
				return false;
		}
	}
	_pResults->PackagingTimeSeconds = (oTimerMSF() - PackageStart) / 1000.0f;
	return true;
}
