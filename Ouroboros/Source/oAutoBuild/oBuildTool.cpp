// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include "oBuildTool.h"
#include <oBasis/oError.h>
#include <oBasis/oScopedPartialTimeout.h>
#include <oBasis/oURI.h>
#include <oBase/timer.h>
#include <oCore/filesystem.h>
#include <oCore/process.h>
#include <oCore/system.h>

using namespace ouro;

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oBUILD_TOOL_TESTING_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oBUILD_TOOL_TESTING_SETTINGS)
	oRTTI_COMPOUND_VERSION(oBUILD_TOOL_TESTING_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oBUILD_TOOL_TESTING_SETTINGS)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, ReSync, oRTTI_OF(bool), "ReSync", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, TimeoutSeconds, oRTTI_OF(uint), "TimeoutSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, CommandLine, oRTTI_OF(ouro_path_string), "CommandLine", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_TESTING_SETTINGS, FailedImageCompares, oRTTI_OF(ouro_path_string), "FailedImageCompares", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oBUILD_TOOL_TESTING_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oBUILD_TOOL_TESTING_SETTINGS)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oBUILD_TOOL_PACKAGING_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oBUILD_TOOL_PACKAGING_SETTINGS)
	oRTTI_COMPOUND_VERSION(oBUILD_TOOL_PACKAGING_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oBUILD_TOOL_PACKAGING_SETTINGS)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_PACKAGING_SETTINGS, TimeoutSeconds, oRTTI_OF(uint), "TimeoutSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_PACKAGING_SETTINGS, CommandLines, oRTTI_OF(std_vector_ouro_path_string), "CommandLines", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oBUILD_TOOL_PACKAGING_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oBUILD_TOOL_PACKAGING_SETTINGS)

static std::regex TestErrorParse("(.+?)\\s*³\\s*(FAILURE|LEAKS)\\s*³(?:.+?)³\\s*(.+?)\\n", std::regex_constants::optimize); // @oooii-kevin: ok static (duplication won't affect correctness)

bool oRunTestingStage(const oBUILD_TOOL_TESTING_SETTINGS& _TestSettings, const char* _BuildRoot, const ouro::event& _CancelEvent, oUnitTestResults* _pResults)
{
	_pResults->TimePassedSeconds = 0.0f;

	// Clean the failed image directory
	ouro::filesystem::remove_directory(ouro::path(_TestSettings.FailedImageCompares));

	path_string command_line = _TestSettings.CommandLine;
	sncatf(command_line, " -z -l %soUnitTests.txt", _BuildRoot);

	// oUnitTests adds stdout/stderr to the filename
	snprintf(_pResults->StdoutLogfile, "%soUnitTests.stdout.txt", _BuildRoot);
	snprintf(_pResults->StderrLogfile, "%soUnitTests.stderr.txt", _BuildRoot);

	ouro::process::info pi;
	pi.command_line = command_line;
	std::shared_ptr<ouro::process> TestProcess = ouro::process::make(pi);

	uint TimeoutMS = _TestSettings.TimeoutSeconds * 1000;
	oScopedPartialTimeout Timer(&TimeoutMS);
	bool Finished = false;
	while(!Finished && TimeoutMS > 0)
	{
		if (_CancelEvent.is_set())
		{
			TestProcess->kill(0);
			return oErrorSetLast(std::errc::operation_canceled);
		}

		Finished = TestProcess->wait_for(std::chrono::milliseconds(500));
		Timer.UpdateTimeout();
	}
	// Always kill the process 
	TestProcess->kill(0);

	// If timed out, wait a little bit otherwise the log files will get a
	// sharing violation when we try to read them.
	if (!Finished)
		std::this_thread::sleep_for(std::chrono::seconds(3));

	_pResults->TimePassedSeconds = (float)((1000*_TestSettings.TimeoutSeconds) - TimeoutMS) / 1000.0f;

	// Copy failed images to the build root
	uri_string FailedImagesSource;
	oURIFromAbsolutePath(FailedImagesSource, _TestSettings.FailedImageCompares);
	uri_string FailedImagesDestination;
	oURIFromAbsolutePath(FailedImagesDestination, _BuildRoot);
	
	ouro::filesystem::copy_all(ouro::path(FailedImagesSource), ouro::path(FailedImagesDestination), ouro::filesystem::copy_option::overwrite_if_exists);
	
	_pResults->FailedImagePath = _BuildRoot;

	// Parse log file
	_pResults->ParseLogfileSucceeded = true;

	scoped_allocation TestResultsBuffer;
	try { TestResultsBuffer = ouro::filesystem::load(ouro::path(_pResults->StdoutLogfile), ouro::filesystem::load_option::text_read); }
	catch (std::exception& e)
	{
		oErrorSetLast(e);
		_pResults->ParseLogfileSucceeded = false;
		oErrorPrefixLast("Failed to load log file %s because ", _pResults->StdoutLogfile.c_str());
		return false;
	}

	std::string TestResults = (const char*)TestResultsBuffer;

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
	float PackageStart = ouro::timer::nowmsf();
	if (!_Settings.CommandLines.empty())
	{
		// FIXME: With CL 23502 the process will hang because of /S on Xcopy. Not capturing STDOUT fixes it
		// xxlstring PackageResponse;
		for (auto& command_line : _Settings.CommandLines)
		{
			int ExitCode = ouro::system::spawn_for(command_line, nullptr, false, _Settings.TimeoutSeconds * 1000);
			if (ExitCode)
				return false;
		}
	}
	_pResults->PackagingTimeSeconds = (ouro::timer::nowmsf() - PackageStart) / 1000.0f;
	return true;
}
