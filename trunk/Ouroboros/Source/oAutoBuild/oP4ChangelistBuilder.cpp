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
#include "oBuildTool.h"
#include "oMSBuild.h"
#include "oAutoBuildOutput.h"
#include <oConcurrency/mutex.h>
#include <oBasis/oINISerialize.h>
#include <oStd/future.h>
#include <oCore/system.h>

using namespace ouro;

static const char* oAUTO_BUILD_ROOT_PATH = "//Root/";

static int oStrFindFirstDiff(const char* _StrSource1, const char* _StrSource2)
{
	const char* origSrc1 = _StrSource1;
	while(*_StrSource1 && *_StrSource2 && *_StrSource1 == *_StrSource2)
	{
		++_StrSource1;
		++_StrSource2;
	}
	if(!*_StrSource1 && !*_StrSource2)
		return -1;
	return static_cast<int>(byte_diff(_StrSource1, origSrc1));
}

struct oBUILD_TOOL_P4_SETTINGS
{
	uri_string Root;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oBUILD_TOOL_P4_SETTINGS)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oBUILD_TOOL_P4_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oBUILD_TOOL_P4_SETTINGS)
	oRTTI_COMPOUND_VERSION(oBUILD_TOOL_P4_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oBUILD_TOOL_P4_SETTINGS)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_P4_SETTINGS, Root, oRTTI_OF(ouro_uri_string), "Root", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oBUILD_TOOL_P4_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oBUILD_TOOL_P4_SETTINGS)

bool oP4CleanSync(int _ChangeList, const char* _SyncPath, const char* _CleanPath = nullptr)
{
	if (_CleanPath)
	{
		windows_path RootPath(_CleanPath);
		RootPath.replace_filename();
		xlstring CmdLine;
		snprintf(CmdLine, "rmdir /S /Q %s", RootPath.c_str());
		::system(CmdLine.c_str());
	}
	return oP4Sync(_ChangeList, _SyncPath, _CleanPath != nullptr);
}

static std::regex EncodedSearch("(%(.+?)%)");

char* oSystemTranslateEnvironmentVariables(char* _StrDestination, size_t _SizeofStrDestination, const char* _RawString)
{
	path_string Current = _RawString;

	const std::cregex_token_iterator end;
	int arr[] = {1,2}; 
	bool NoTranslations = true;
	for ( std::cregex_token_iterator VecTok(_RawString, _RawString + strlen(_RawString), EncodedSearch, arr); VecTok != end; ++VecTok )
	{
		auto Replace = VecTok->str();
		++VecTok;
		auto EnvVariable = VecTok->str();
		path_string TranslatedVariable;
		ouro::system::getenv(TranslatedVariable, EnvVariable.c_str());
		replace(_StrDestination, _SizeofStrDestination, Current, Replace.c_str(), TranslatedVariable.c_str());
		Current = _StrDestination;
		NoTranslations = false;
	}
	if( NoTranslations )
		strlcpy(_StrDestination, Current, _SizeofStrDestination);

	return _StrDestination;
}
template<size_t size> char* oSystemTranslateEnvironmentVariables(char (&_Value)[size], const char* _Name) { return oSystemTranslateEnvironmentVariables(_Value, size, _Name); }
template<size_t capacity> char* oSystemTranslateEnvironmentVariables(ouro::fixed_string<char, capacity>& _Value, const char* _Name) { return oSystemTranslateEnvironmentVariables(_Value, _Value.capacity(), _Name); }

static const int NUM_BUILDS_TO_SERVE = 10;

// Helper class that terminates all child processes of this process.
// Assumes a choke point where both the main process and a single
// worker thread are not making any system calls
class oChildProcessTerminator
{
public:
	oChildProcessTerminator()
	{
		AssertIsMain();
		ProcessID = ouro::this_process::get_id();
		Mutex.lock();
	}
	void Terminate()
	{
		oConcurrency::lock_guard<oConcurrency::mutex> Lock(Mutex);
		ouro::process::terminate_children(ProcessID, 0, true);
	}

	void MainThreadYield(uint _YieldMS)
	{
		AssertIsMain();
		Mutex.unlock();
		oStd::this_thread::sleep_for(oStd::chrono::milliseconds(_YieldMS));
		Mutex.lock();
	}
	void MainThreadRelease()
	{
		Mutex.unlock();
	}
private:
	oConcurrency::mutex Mutex;
	ouro::process::id ProcessID;

	void AssertIsMain()
	{
		oASSERT(ouro::this_process::get_main_thread_id() == oStd::this_thread::get_id(), "Must be called from main thread");
	}
};

class oP4ChangelistBuilderImpl : public oP4ChangelistBuilder
{
public:
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oP4ChangelistBuilderImpl(const ini& _INI, const char* _LogRoot, int _ServerPort, bool *_pSuccess)
		: ServerPort(_ServerPort)
		, LastCL(ouro::invalid)
		, StartBuildMS(0)
		, CurrentBuildInfoValid(false)
		, CurrentBuildActive(false)
		, LastBuildMS(0)
		, LastDailyBuildMS(0)
	{
		ini::section section = _INI.first_section();
		while (section)
		{
			const char* pSectionName = _INI.section_name(section);
			if(0 == _stricmp(pSectionName, "Perforce"))
			{
				oINIReadCompound(&P4Settings, oRTTI_OF(oBUILD_TOOL_P4_SETTINGS), _INI, section, false);
			}
			else if(0 == _stricmp(pSectionName, "Email"))
			{
				oINIReadCompound(&EmailSettings, oRTTI_OF(oAutoBuildEmailSettings), _INI, section, false);
			}
			else if(0 == _stricmp(pSectionName, "MSBuild"))
			{
				oINIReadCompound(&BuildSettings, oRTTI_OF(oMSBUILD_SETTINGS), _INI, section, false);
				auto Temp = BuildSettings.ToolPath;
				oSystemTranslateEnvironmentVariables(BuildSettings.ToolPath, Temp);
			}
			else if(0 == _stricmp(pSectionName, "TestingChangelist"))
			{
				oINIReadCompound(&TestSettings, oRTTI_OF(oBUILD_TOOL_TESTING_SETTINGS), _INI, section, false);
			}
			else if(0 == _stricmp(pSectionName, "TestingDaily"))
			{
				oINIReadCompound(&TestSettingsDaily, oRTTI_OF(oBUILD_TOOL_TESTING_SETTINGS), _INI, section, false);
			}
			else if(0 == _stricmp(pSectionName, "Packaging"))
			{
				oINIReadCompound(&PackagingSettings, oRTTI_OF(oBUILD_TOOL_PACKAGING_SETTINGS), _INI, section, false);
			}
			section = _INI.next_section(section);
		}

		clean_path(LogRoot, _LogRoot);

		// Patch up all paths relative to the perforce root
		if (P4Settings.Root[P4Settings.Root.length()-1] != '/')
			strlcat(P4Settings.Root, "/");

		if (BuildSettings.Solution.empty())
		{
			oErrorSetLast(std::errc::invalid_argument, "Missing solution");
			return;
		}

		path_string Temp;
		replace(Temp, BuildSettings.Solution, oAUTO_BUILD_ROOT_PATH, P4Settings.Root);

		if(!oP4GetClientPath(BuildSettings.Solution.c_str(), Temp))
			return; // Sets last error

		// The directory of the solution will determine our root
		path FileRoot(BuildSettings.Solution);
		FileRoot.replace_filename();

		// Patch everything else relative to this
		std::function<void(path_string& _PatchPath)> RootPatcher = 
			[&](path_string& _PatchPath)
		{
			Temp = _PatchPath;
			replace(_PatchPath, Temp, oAUTO_BUILD_ROOT_PATH, FileRoot);
		};
		RootPatcher(TestSettings.CommandLine);
		RootPatcher(TestSettings.FailedImageCompares);
		RootPatcher(TestSettingsDaily.CommandLine);
		RootPatcher(TestSettingsDaily.FailedImageCompares);
		oFOR(auto& command_line, PackagingSettings.CommandLines)
		{
			RootPatcher(command_line);
		}

		sncatf(P4Settings.Root, "...");

		ScanBuildLogsFolder();
		UpdateBuildStatistics();

		TryAddingChangelist(oP4GetCurrentChangelist(P4Settings.Root));

		*_pSuccess = true;
	}
	
	void TryNextBuild(int _DailyBuildHour) override;
	
	int GetCount() const override;

	void ReportWorking(std::function<void(const oP4ChangelistBuilder::ChangeInfo& _Change, int _RemainingMS, int _PercentageDone)> _Reporter) const override;
	void ReportBuilt(std::function<void(const std::list<oP4ChangelistBuilder::ChangeInfo> & _Changes)> _Reporter) const override;
	void ReportLastSpecialBuild(std::function<void(const char* _pName, bool _Success, const char* _pLastSuccesful)> _Reporter) const override;

	void MainThreadYield(uint _Milleseconds) override
	{
		Terminator.MainThreadYield(_Milleseconds);
	}

	void Cancel() override
	{
		Terminator.MainThreadRelease();
		CancelEvent.set();
		if (CurrentBuildActive)
			CurrentBuild.wait();
	}

private:
	bool ParseAutoBuildResults(int _CL, ChangeInfo* _pInfo);
	bool ParseAutoBuildResultsSpecialBuild(const char* _pName, ChangeInfo* _pInfo);
	bool ParseAutoBuildResults(const char* _pAbsolutePath, ChangeInfo* _pInfo);

	void ScanBuildLogsFolder();
	void UpdateBuildStatistics();

	bool WasChangelistAlreadyAdded(int _Changelist, bool _IsDaily = false);
	bool WasChangelistAlreadyBuilt(int _Changelist, bool _IsDaily = false);
	void TryAddingChangelist(int _Changelist, bool _IsDaily = false);

	ChangeInfo* GetNextBuild();
	void UpdateBuildProgress(ChangeInfo* _pBuild, const char* _Stage);
	void FinishBuild(ChangeInfo* _pBuild);

	void BuildNextBuild();

	oRefCount Refcount;
	oChildProcessTerminator Terminator;
	oConcurrency::event CancelEvent;
	oConcurrency::shared_mutex Mutex;
	int ServerPort;

	// Settings
	path_string LogRoot;
	oAutoBuildEmailSettings EmailSettings;
	oBUILD_TOOL_P4_SETTINGS P4Settings;
	oMSBUILD_SETTINGS BuildSettings;
	oBUILD_TOOL_TESTING_SETTINGS TestSettings;
	oBUILD_TOOL_TESTING_SETTINGS TestSettingsDaily;
	oBUILD_TOOL_PACKAGING_SETTINGS PackagingSettings;

	// Pending
	std::list<ChangeInfo> NextBuildInfos;
	int LastCL;

	// Current
	int StartBuildMS; // time stamp
	bool CurrentBuildInfoValid;
	ChangeInfo CurrentBuildInfo;
	oStd::future<void> CurrentBuild;
	bool CurrentBuildActive; // For tracking the CurrentBuild future

	// Finished
	std::list<ChangeInfo> FinishedBuildInfos;
	std::list<ChangeInfo> FinishedDailyBuildInfos;
	ChangeInfo LastSuccesfulBuildInfo;
	ChangeInfo LastSuccesfulDailyBuildInfo;
	int LastBuildMS; // duration
	uint LastDailyBuildMS; // time stamp

	// Statistics
	int AverageBuildTimeMS;
	int AverageDailyBuildTimeMS;
};

bool oChangelistManagerCreate(const ini& _INI, const char* _LogRoot, int _ServerPort, oP4ChangelistBuilder** _ppChangelistManager)
{
	bool success = false;
	oCONSTRUCT(_ppChangelistManager, oP4ChangelistBuilderImpl(_INI, _LogRoot, _ServerPort, &success));
	return success;
}

void oP4ChangelistBuilderImpl::ScanBuildLogsFolder()
{
	path FileWildCard(LogRoot);
	FileWildCard /= "*";

	std::vector<sstring> SpecialBuildPaths;
	std::vector<sstring> SuccesfulBuildPaths;
	ouro::filesystem::enumerate(FileWildCard, 
	[&](const path& _FullPath, const ouro::filesystem::file_status& _Status, unsigned long long _Size)->bool
	{
		path_string PossibleBuild;
		snprintf(PossibleBuild, "%s/index.html", _FullPath.c_str());
		if(ouro::filesystem::exists(path(PossibleBuild)))
		{
			char* pFileName = PossibleBuild.c_str() + oStrFindFirstDiff(FileWildCard, PossibleBuild);

			if(0 == strncmp(pFileName, oAUTO_BUILD_SPECIAL_PREFIX, 5))
				SpecialBuildPaths.push_back(pFileName);
			else
				SuccesfulBuildPaths.push_back(pFileName);
		}
		return true;
	});

	std::reverse(SuccesfulBuildPaths.begin(), SuccesfulBuildPaths.end());

	bool foundLastSuccessful = false;
	oFOR(auto& _Path, SuccesfulBuildPaths)
	{
		path_string TempPath = _Path;
		strchr(TempPath.c_str(), '/')[0] = 0;
		ChangeInfo Info;
		Info.IsDaily = false;
		if(from_string(&Info.CL, TempPath))
		{
			Info.Succeeded = ParseAutoBuildResults(Info.CL, &Info);
			FinishedBuildInfos.push_back(Info);
			if (!foundLastSuccessful && Info.Succeeded)
			{
				LastSuccesfulBuildInfo = Info;
				foundLastSuccessful = true;
			}
		}
	}

	// Only get user names for the amount that's going to be served,
	// because it's slow and spawning p4.exe every time.
	// If the REST API will ask for more than that in the future we could
	// then on the spot fill in any missing user names.
	int count = 1;
	for(auto FinishedBuildInfo = FinishedBuildInfos.begin(); FinishedBuildInfo != FinishedBuildInfos.end(); ++FinishedBuildInfo)
	{
		oP4GetChangelistUser(FinishedBuildInfo->UserName.c_str(), FinishedBuildInfo->UserName.capacity(), FinishedBuildInfo->CL);
		oP4GetChangelistDate(FinishedBuildInfo->Date.c_str(), FinishedBuildInfo->Date.capacity(), FinishedBuildInfo->CL);
		if (count++ == NUM_BUILDS_TO_SERVE) break;
	}

	std::reverse(SpecialBuildPaths.begin(), SpecialBuildPaths.end());

	foundLastSuccessful = false;
	oFOR(auto& _Path, SpecialBuildPaths)
	{
		path_string TempPath = _Path;
		strchr(TempPath.c_str(), '/')[0] = 0;
		ChangeInfo Info;
		Info.IsDaily = true;
		Info.CL = ouro::invalid;
		Info.UserName = TempPath;
		Info.Succeeded = ParseAutoBuildResultsSpecialBuild(TempPath, &Info);
		FinishedDailyBuildInfos.push_back(Info);
		if (!foundLastSuccessful && Info.Succeeded)
		{
			LastSuccesfulDailyBuildInfo = Info;
			foundLastSuccessful = true;
		}
	}
}

void oP4ChangelistBuilderImpl::UpdateBuildStatistics()
{
	AverageBuildTimeMS = 0;
	AverageDailyBuildTimeMS = 0;

	__int64 TotalBuildTimeMS = 0;
	__int64 SuccessfulBuilds = 0;
	for(auto FinishedBuildInfo = FinishedBuildInfos.begin(); FinishedBuildInfo != FinishedBuildInfos.end() && SuccessfulBuilds < NUM_BUILDS_TO_SERVE; ++FinishedBuildInfo)
	{
		if (FinishedBuildInfo->Succeeded)
		{
			TotalBuildTimeMS += (FinishedBuildInfo->BuildTime + FinishedBuildInfo->TestTime + FinishedBuildInfo->PackTime) * 1000;
			SuccessfulBuilds++;
		}
	}
	
	if (SuccessfulBuilds)
		AverageBuildTimeMS = static_cast<int>(TotalBuildTimeMS / SuccessfulBuilds);

	TotalBuildTimeMS = 0;
	SuccessfulBuilds = 0;
	for(auto FinishedBuildInfo = FinishedDailyBuildInfos.begin(); FinishedBuildInfo != FinishedDailyBuildInfos.end() && SuccessfulBuilds < NUM_BUILDS_TO_SERVE; ++FinishedBuildInfo)
	{
		if (FinishedBuildInfo->Succeeded)
		{
			TotalBuildTimeMS += (FinishedBuildInfo->BuildTime + FinishedBuildInfo->TestTime + FinishedBuildInfo->PackTime) * 1000;
			SuccessfulBuilds++;
		}
	}

	if (SuccessfulBuilds)
		AverageDailyBuildTimeMS = static_cast<int>(TotalBuildTimeMS / SuccessfulBuilds);
}

bool oP4ChangelistBuilderImpl::WasChangelistAlreadyAdded(int _Changelist, bool _IsDaily /*= false*/)
{
	if (_IsDaily)
	{
		uint CurrentTimeMS = ouro::timer::now_ms();
		return (0 != LastDailyBuildMS && ((CurrentTimeMS - LastDailyBuildMS) < (2 * 60 * 60 * 1000/*2 hours ms*/)));
	}
	else
	{
		// @oooii-jeffrey: Was !=
		return _Changelist <= LastCL;
	}
}

bool oP4ChangelistBuilderImpl::WasChangelistAlreadyBuilt(int _Changelist, bool _IsDaily /*= false*/)
{
	// Daily builds are always built, as it goes through different testing as
	// the regular builds.
	if (!_IsDaily)
	{
		// This function assumes the Mutex was locked by the caller
		oFOR(auto BuildInfo, FinishedBuildInfos)
		{
			if (BuildInfo.CL == _Changelist)
				return true;
		}
	}
	return false;
}

void oP4ChangelistBuilderImpl::TryAddingChangelist(int _Changelist, bool _IsDaily /*= false*/)
{
	if (_Changelist == ouro::invalid)
		return;

	if (WasChangelistAlreadyAdded(_Changelist, _IsDaily))
		return;

	ChangeInfo NextBuild;
	NextBuild.IsDaily = _IsDaily;
	NextBuild.CL = _Changelist;
	NextBuild.Succeeded = false;
	NextBuild.Stage = "Pending";
	NextBuild.BuildTime = 0;
	NextBuild.TestTime = 0;
	NextBuild.PackTime = 0;
	oP4GetChangelistDate(NextBuild.Date.c_str(), NextBuild.Date.capacity(), _Changelist);

	if (_IsDaily)
	{
		ouro::date CurrentDate;
		ouro::system::now(&CurrentDate);
		CurrentDate = ouro::system::to_local(CurrentDate);

		sstring DateStr;
		strftime(DateStr, "%Y%m%d", CurrentDate);

		snprintf(NextBuild.UserName, "%s_%s_%d", oAUTO_BUILD_SPECIAL_PREFIX, DateStr.c_str(), _Changelist);

		oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(Mutex);
		NextBuildInfos.push_back(NextBuild);
		LastDailyBuildMS = ouro::timer::now_ms();
	}
	else
	{
		oP4GetChangelistUser(NextBuild.UserName.c_str(), NextBuild.UserName.capacity(), _Changelist);
	
		oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(Mutex);
		if (!WasChangelistAlreadyBuilt(_Changelist))
			NextBuildInfos.push_back(NextBuild);
		LastCL = _Changelist;
	}
}


void oP4ChangelistBuilderImpl::TryNextBuild(int _DailyBuildHour)
{
	// Check p4 for new builds. This can be ouro::invalid if we lose our connection.
	int NextCL = oP4GetNextChangelist(LastCL, P4Settings.Root);
	TryAddingChangelist(NextCL);

	// We only can run the daily build if we're not running a remote session as 
	// this will do more extensive testing
	if(!ouro::system::is_remote_session())
	{
		ouro::date CurrentDate;
		ouro::system::now(&CurrentDate);
		CurrentDate = ouro::system::to_local(CurrentDate);

		if (_DailyBuildHour == CurrentDate.hour && !WasChangelistAlreadyAdded(ouro::invalid, true))
		{
			TryAddingChangelist(LastSuccesfulBuildInfo.CL, true);
		}
	}

	if (CurrentBuildActive)
	{
		if (CurrentBuild.is_ready())
		{
			CurrentBuild.wait();
			CurrentBuildActive = false;
		}
	}

	if (!CurrentBuildActive && NextBuildInfos.size() > 0)
	{
		CurrentBuild = oStd::async(std::bind(&oP4ChangelistBuilderImpl::BuildNextBuild, this));
		CurrentBuildActive = true;
	}
}

oP4ChangelistBuilder::ChangeInfo* oP4ChangelistBuilderImpl::GetNextBuild()
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(Mutex);
	oASSERT(!NextBuildInfos.empty(), "Popping an empty list");
	StartBuildMS = ouro::timer::now_ms();
	CurrentBuildInfo = std::move(NextBuildInfos.front());
	CurrentBuildInfoValid = true;
	NextBuildInfos.pop_front();
	CurrentBuildInfo.Stage = "Started";
	return &CurrentBuildInfo;
}

void oP4ChangelistBuilderImpl::UpdateBuildProgress(oP4ChangelistBuilder::ChangeInfo* _pBuild, const char* _Stage)
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(Mutex);
	_pBuild->Stage = _Stage;
}

void oP4ChangelistBuilderImpl::FinishBuild(oP4ChangelistBuilder::ChangeInfo* _pBuild)
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(Mutex);
	LastBuildMS = (ouro::timer::now_ms() - StartBuildMS);

	if (_pBuild->IsDaily)
	{
		_pBuild->Succeeded = ParseAutoBuildResultsSpecialBuild(_pBuild->UserName.c_str(), _pBuild);

		FinishedDailyBuildInfos.push_front(*_pBuild);
		if (_pBuild->Succeeded)
			LastSuccesfulDailyBuildInfo = *_pBuild;
	}
	else
	{
		_pBuild->Succeeded = ParseAutoBuildResults(_pBuild->CL, _pBuild);
		FinishedBuildInfos.push_front(*_pBuild);
		if (_pBuild->Succeeded)
			LastSuccesfulBuildInfo = *_pBuild;
	}

	CurrentBuildInfoValid = false;
	UpdateBuildStatistics();
}



void oP4ChangelistBuilderImpl::BuildNextBuild()
{
	Terminator.Terminate();

	ChangeInfo* CurrentBuild = GetNextBuild();

	if (CancelEvent.is_set())
		return;

	const oBUILD_TOOL_TESTING_SETTINGS& CurrentTestSettings = CurrentBuild->IsDaily ? TestSettingsDaily : TestSettings;

	oAutoBuildResults results;
	results.IsDailyBuild = CurrentBuild->IsDaily;
	results.ChangeList = CurrentBuild->CL;

	if (CurrentBuild->IsDaily)
		results.BuildName = CurrentBuild->UserName;
	else
		to_string(results.BuildName, CurrentBuild->CL);

	UpdateBuildProgress(CurrentBuild, "P4 Sync");

	// When re-syncing we blow away the old directory then force sync the directory again
	if (!oP4CleanSync(CurrentBuild->CL, P4Settings.Root.c_str(), CurrentTestSettings.ReSync ? BuildSettings.Solution.c_str() : nullptr))
	{
		lstring Error;
		snprintf(Error, "P4 failed to sync with %s", oErrorGetLastString());
		oEmailAdminAndStop(EmailSettings, Error, CurrentBuild->CL, false);
	}

	path Path(LogRoot);
	Path /= results.BuildName;
	ouro::filesystem::create_directories(Path.parent_path());
	results.OutputFolder = Path;

	if (CancelEvent.is_set())
		return;

	// First build the app
	UpdateBuildProgress(CurrentBuild, "Building");
	oMSBuildAndLog(BuildSettings, Path, CancelEvent, &results.BuildResults);
	if (results.BuildResults.BuildSucceeded)
	{
		if (CancelEvent.is_set())
			return;

		// Run unit tests
		UpdateBuildProgress(CurrentBuild, "Unit testing");
		if (!oRunTestingStage(CurrentTestSettings, Path, CancelEvent, &results.TestResults))
		{
			oErrorPrefixLast("Failed to start test process %s because ", CurrentTestSettings.CommandLine);
			if (!CancelEvent.is_set())
				oEmailAdminAndStop(EmailSettings, oErrorGetLastString(), CurrentBuild->CL, false);
			return;
		}

		if (CancelEvent.is_set())
			return;

		// Run packaging
		UpdateBuildProgress(CurrentBuild, "Packaging");
		if (!oRunPackagingStage(PackagingSettings, &results.PackagingResults))
		{
			oErrorPrefixLast("Packaging failed with ");
			if (!CancelEvent.is_set())
				oEmailAdminAndStop(EmailSettings, oErrorGetLastString(), CurrentBuild->CL, false);
			return;
		}
	}

	if (CancelEvent.is_set())
		return;

	UpdateBuildProgress(CurrentBuild, "Formatting results");

	oAutoBuildOutputResults(EmailSettings, ServerPort, results);
	
	UpdateBuildProgress(CurrentBuild, "Finished");

	FinishBuild(CurrentBuild);
}



int oP4ChangelistBuilderImpl::GetCount() const
{
	oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(Mutex);
	return static_cast<int>(NextBuildInfos.size() + (uint)CurrentBuildActive);
}

void oP4ChangelistBuilderImpl::ReportWorking(std::function<void(const oP4ChangelistBuilder::ChangeInfo& _Change, int _RemainingMS, int _PercentageDone)> _Reporter) const
{
	oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(Mutex);

	// First calculate the time left for the current build
	int TimePastMS = ouro::timer::now_ms() - StartBuildMS;
	int RemainingMS = 0;
	if (CurrentBuildInfoValid)
		RemainingMS = (CurrentBuildInfo.IsDaily ? AverageDailyBuildTimeMS : AverageBuildTimeMS) - TimePastMS;

	// Clamp that to 0
	int TotalBuildTimeLeft = (RemainingMS > 0) ? RemainingMS : 0;

	// Add the estimated build times for all the pending builds
	for(auto NextBuildInfo = NextBuildInfos.rbegin(); NextBuildInfo != NextBuildInfos.rend(); ++NextBuildInfo)
		TotalBuildTimeLeft += NextBuildInfo->IsDaily ? AverageDailyBuildTimeMS : AverageBuildTimeMS;

	// Report all the pending builds first in reversed order
	for(auto NextBuildInfo = NextBuildInfos.rbegin(); NextBuildInfo != NextBuildInfos.rend(); ++NextBuildInfo)
	{
		_Reporter(*NextBuildInfo, TotalBuildTimeLeft, 0);

		TotalBuildTimeLeft -= NextBuildInfo->IsDaily ? AverageDailyBuildTimeMS : AverageBuildTimeMS;
	}

	// Report the current build with estimated percentage done
	if (CurrentBuildInfoValid)
	{
		int averageBuildTimeMS = CurrentBuildInfo.IsDaily ? AverageDailyBuildTimeMS : AverageBuildTimeMS;
		float progress = (RemainingMS < averageBuildTimeMS && RemainingMS > 0) ? (float)TimePastMS / (float)averageBuildTimeMS : 0.98f;
		int PercentageDone = (int)(progress * 100.0f);

		_Reporter(CurrentBuildInfo, RemainingMS, PercentageDone);
	}
}

void oP4ChangelistBuilderImpl::ReportBuilt(std::function<void(const std::list<oP4ChangelistBuilder::ChangeInfo> & _Changes)> _Reporter) const
{
	// TODO: Take ownership over the iteration like in ReportWorking, then FinishedBuildInfos doesn't need to be in reversed order anymore
	oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(Mutex);
	_Reporter(FinishedBuildInfos);
}

void oP4ChangelistBuilderImpl::ReportLastSpecialBuild(std::function<void(const char* _pName, bool _Success, const char* _pLastSuccesful)> _Reporter) const
{
	oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(Mutex);
	if (FinishedDailyBuildInfos.size())
	{
		_Reporter(FinishedDailyBuildInfos.front().UserName.c_str(), FinishedDailyBuildInfos.front().Succeeded, LastSuccesfulDailyBuildInfo.UserName.c_str());
	}
}

bool oP4ChangelistBuilderImpl::ParseAutoBuildResults(int _CL, ChangeInfo* _pInfo)
{
	path_string PathToCL;
	snprintf(PathToCL, "%s%d/index.html", LogRoot.c_str(), _CL);
	return ParseAutoBuildResults(PathToCL, _pInfo);
}

bool oP4ChangelistBuilderImpl::ParseAutoBuildResultsSpecialBuild(const char* _pName, ChangeInfo* _pInfo)
{
	path_string AbsolutePath;
	snprintf(AbsolutePath, "%s%s/index.html", LogRoot.c_str(), _pName);
	return ParseAutoBuildResults(AbsolutePath, _pInfo);
}

bool oP4ChangelistBuilderImpl::ParseAutoBuildResults(const char* _pAbsolutePath, ChangeInfo* _pInfo)
{
	_pInfo->BuildTime = 0;
	_pInfo->TestTime = 0;
	_pInfo->PackTime = 0;

	std::shared_ptr<xml> XML = oXMLLoad(_pAbsolutePath);
	if (!XML)
		return false;

	auto TitleNode = XML->first_child(XML->root(), "title");
	if (!TitleNode)
		return false;

	sstring Title = XML->node_value(TitleNode);
	if (Title.empty())
		return false;

	bool result = (strstr(Title, oBUILD_TOOL_FAIL_TITLE) == nullptr);

	auto BodyNode = XML->first_child(XML->root(), "body");
	if (!BodyNode)
		return result;

	for (auto node = XML->first_child(BodyNode); node; node = XML->next_sibling(node))
	{
		const char* nodevalue = XML->node_value(node);

		if (strstr(nodevalue, "Build"))
		{
			const char* tok = strstr(nodevalue, "taking ");
			if (tok)
			{
				tok += strlen("taking ");
				double time = atof(tok);
				_pInfo->BuildTime = (int)time;
			}
		}
		else if (strstr(nodevalue, "Testing"))
		{
			const char* tok = strstr(nodevalue, "taking ");
			if (tok)
			{
				tok += strlen("taking ");
				double time = atof(tok);
				_pInfo->TestTime = (int)time;
			}
		}
		else if (strstr(nodevalue, "Packaging"))
		{
			const char* tok = strstr(nodevalue, "taking ");
			if (tok)
			{
				tok += strlen("taking ");
				double time = atof(tok);
				_pInfo->PackTime = (int)time;
			}
		}
	}

	return result;
}
