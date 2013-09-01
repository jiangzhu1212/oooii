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
#include <oStd/oStdFuture.h>

static const char* oAUTO_BUILD_ROOT_PATH = "//Root/";

struct oBUILD_TOOL_P4_SETTINGS
{
	oStd::uri_string Root;
};
oRTTI_COMPOUND_DECLARATION(oRTTI_CAPS_NONE, oBUILD_TOOL_P4_SETTINGS)

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oBUILD_TOOL_P4_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oBUILD_TOOL_P4_SETTINGS)
	oRTTI_COMPOUND_VERSION(oBUILD_TOOL_P4_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oBUILD_TOOL_P4_SETTINGS)
		oRTTI_COMPOUND_ATTR(oBUILD_TOOL_P4_SETTINGS, Root, oRTTI_OF(ostd_uri_string), "Root", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oBUILD_TOOL_P4_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oBUILD_TOOL_P4_SETTINGS)

bool oP4CleanSync(int _ChangeList, const char* _SyncPath, const char* _CleanPath = nullptr)
{
	if (_CleanPath)
	{
		oStd::path_string RootPath;
		oStd::replace(RootPath, _CleanPath, "/", "\\");
		oTrimFilename(RootPath);
		oStd::xlstring CmdLine;
		oPrintf(CmdLine, "rmdir /S /Q %s", RootPath.c_str());
		system(CmdLine.c_str());
	}
	return oP4Sync(_ChangeList, _SyncPath, _CleanPath != nullptr);
}

static const int NUM_BUILDS_TO_SERVE = 10;

// Helper class that terminates all child processes of this process.
// Assumes a choke point where both the main process and a single
// worker thread are not making any system calls
class oChildProcesTerminator
{
public:
	oChildProcesTerminator()
	{
		AssertIsMain();
		ProcessID = oProcessGetCurrentID();
		Mutex.lock();
	}
	void Terminate()
	{
		oConcurrency::lock_guard<oConcurrency::mutex> Lock(Mutex);
		oProcessTerminateChildren(ProcessID, 0, true);
	}

	void MainThreadYield(uint _YieldMS)
	{
		AssertIsMain();
		Mutex.unlock();
		oSleep(_YieldMS);
		Mutex.lock();
	}
	void MainThreadRelease()
	{
		Mutex.unlock();
	}
private:
	oConcurrency::mutex Mutex;
	uint ProcessID;

	void AssertIsMain()
	{
		oASSERT(oConcurrency::main_thread::get_id() == oStd::this_thread::get_id(), "Must be called from main thread");
	}
};

class oP4ChangelistBuilderImpl : public oP4ChangelistBuilder
{
public:
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oP4ChangelistBuilderImpl(const oStd::ini& _INI, const char* _LogRoot, int _ServerPort, bool *_pSuccess)
		: ServerPort(_ServerPort)
		, LastCL(oInvalid)
		, StartBuildMS(0)
		, CurrentBuildInfoValid(false)
		, CurrentBuildActive(false)
		, LastBuildMS(0)
		, LastDailyBuildMS(0)
	{
		oStd::ini::section section = _INI.first_section();
		while (section)
		{
			const char* pSectionName = _INI.section_name(section);
			if(0 == oStricmp(pSectionName, "Perforce"))
			{
				oINIReadCompound(&P4Settings, oRTTI_OF(oBUILD_TOOL_P4_SETTINGS), _INI, section, false);
			}
			else if(0 == oStricmp(pSectionName, "Email"))
			{
				oINIReadCompound(&EmailSettings, oRTTI_OF(oAutoBuildEmailSettings), _INI, section, false);
			}
			else if(0 == oStricmp(pSectionName, "MSBuild"))
			{
				oINIReadCompound(&BuildSettings, oRTTI_OF(oMSBUILD_SETTINGS), _INI, section, false);
				auto Temp = BuildSettings.ToolPath;
				oSystemTranslateEnvironmentVariables(BuildSettings.ToolPath, Temp);
			}
			else if(0 == oStricmp(pSectionName, "TestingChangelist"))
			{
				oINIReadCompound(&TestSettings, oRTTI_OF(oBUILD_TOOL_TESTING_SETTINGS), _INI, section, false);
			}
			else if(0 == oStricmp(pSectionName, "TestingDaily"))
			{
				oINIReadCompound(&TestSettingsDaily, oRTTI_OF(oBUILD_TOOL_TESTING_SETTINGS), _INI, section, false);
			}
			else if(0 == oStricmp(pSectionName, "Packaging"))
			{
				oINIReadCompound(&PackagingSettings, oRTTI_OF(oBUILD_TOOL_PACKAGING_SETTINGS), _INI, section, false);
			}
			section = _INI.next_section(section);
		}

		oCleanPath(LogRoot, _LogRoot);

		// Patch up all paths relative to the perforce root
		oEnsureSeparator(P4Settings.Root);

		if (BuildSettings.Solution.empty())
		{
			oErrorSetLast(std::errc::invalid_argument, "Missing solution");
			return;
		}

		oStd::path_string Temp;
		oStd::replace(Temp, BuildSettings.Solution, oAUTO_BUILD_ROOT_PATH, P4Settings.Root);

		if(!oP4GetClientPath(BuildSettings.Solution.c_str(), Temp))
			return; // Sets last error

		// The directory of the solution will determine our root
		oStd::path_string FileRoot;
		FileRoot = BuildSettings.Solution.c_str();
		oTrimFilename(FileRoot.c_str());

		// Patch everything else relative to this
		oFUNCTION<void(oStd::path_string& _PatchPath)> RootPatcher = 
			[&](oStd::path_string& _PatchPath)
		{
			Temp = _PatchPath;
			oStd::replace(_PatchPath, Temp, oAUTO_BUILD_ROOT_PATH, FileRoot);
		};
		RootPatcher(TestSettings.CommandLine);
		RootPatcher(TestSettings.FailedImageCompares);
		RootPatcher(TestSettingsDaily.CommandLine);
		RootPatcher(TestSettingsDaily.FailedImageCompares);
		oFOR(auto& command_line, PackagingSettings.CommandLines)
		{
			RootPatcher(command_line);
		}

		oStrAppendf(P4Settings.Root, "...");

		ScanBuildLogsFolder();
		UpdateBuildStatistics();

		TryAddingChangelist(oP4GetCurrentChangelist(P4Settings.Root));

		*_pSuccess = true;
	}
	
	void TryNextBuild(int _DailyBuildHour) override;
	
	int GetCount() const override;

	void ReportWorking(oFUNCTION<void(const oP4ChangelistBuilder::ChangeInfo& _Change, int _RemainingMS, int _PercentageDone)> _Reporter) const override;
	void ReportBuilt(oFUNCTION<void(const std::list<oP4ChangelistBuilder::ChangeInfo> & _Changes)> _Reporter) const override;
	void ReportLastSpecialBuild(oFUNCTION<void(const char* _pName, bool _Success, const char* _pLastSuccesful)> _Reporter) const override;

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
	oChildProcesTerminator Terminator;
	oConcurrency::event CancelEvent;
	oConcurrency::shared_mutex Mutex;
	int ServerPort;

	// Settings
	oStd::path_string LogRoot;
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

bool oChangelistManagerCreate(const oStd::ini& _INI, const char* _LogRoot, int _ServerPort, oP4ChangelistBuilder** _ppChangelistManager)
{
	bool success = false;
	oCONSTRUCT(_ppChangelistManager, oP4ChangelistBuilderImpl(_INI, _LogRoot, _ServerPort, &success));
	return success;
}

void oP4ChangelistBuilderImpl::ScanBuildLogsFolder()
{
	oStd::path_string FileWildCard;
	oPrintf(FileWildCard, "%s*", LogRoot);

	std::vector<oStd::sstring> SpecialBuildPaths;
	std::vector<oStd::sstring> SuccesfulBuildPaths;
	oFileEnum(FileWildCard,
		[&](const char* _pFullPath, const oSTREAM_DESC& _Desc)->bool
	{
		oStd::path_string PossibleBuild;
		oPrintf(PossibleBuild, "%s/index.html", _pFullPath);
		if(oFileExists(PossibleBuild))
		{
			char* pFileName = PossibleBuild.c_str() + oStrFindFirstDiff(FileWildCard, PossibleBuild);

			if(0 == oStrncmp(pFileName, oAUTO_BUILD_SPECIAL_PREFIX, 5))
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
		oStd::path_string TempPath = _Path;
		strchr(TempPath.c_str(), '/')[0] = 0;
		ChangeInfo Info;
		Info.IsDaily = false;
		if(oStd::from_string(&Info.CL, TempPath))
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
		oStd::path_string TempPath = _Path;
		strchr(TempPath.c_str(), '/')[0] = 0;
		ChangeInfo Info;
		Info.IsDaily = true;
		Info.CL = oInvalid;
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
		AverageBuildTimeMS = oInt(TotalBuildTimeMS / SuccessfulBuilds);

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
		AverageDailyBuildTimeMS = oInt(TotalBuildTimeMS / SuccessfulBuilds);
}

bool oP4ChangelistBuilderImpl::WasChangelistAlreadyAdded(int _Changelist, bool _IsDaily /*= false*/)
{
	if (_IsDaily)
	{
		uint CurrentTimeMS = oTimerMS();
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
	if (_Changelist == oInvalid)
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
		oStd::date CurrentDate;
		oSystemGetDate(&CurrentDate);
		oSystemDateToLocal(CurrentDate, &CurrentDate);

		oStd::sstring DateStr;
		oStd::strftime(DateStr, "%Y%m%d", CurrentDate);

		oPrintf(NextBuild.UserName, "%s_%s_%d", oAUTO_BUILD_SPECIAL_PREFIX, DateStr.c_str(), _Changelist);

		oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(Mutex);
		NextBuildInfos.push_back(NextBuild);
		LastDailyBuildMS = oTimerMS();
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
	// Check p4 for new builds. This can be oInvalid if we lose our connection.
	int NextCL = oP4GetNextChangelist(LastCL, P4Settings.Root);
	TryAddingChangelist(NextCL);

	// We only can run the daily build if we're not running a remote session as 
	// this will do more extensive testing
	if(!oSystemIsRemote())
	{
		oStd::date CurrentDate;
		oSystemGetDate(&CurrentDate);
		oSystemDateToLocal(CurrentDate, &CurrentDate);

		if (_DailyBuildHour == CurrentDate.hour && !WasChangelistAlreadyAdded(oInvalid, true))
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
		CurrentBuild = oStd::async(oBIND(&oP4ChangelistBuilderImpl::BuildNextBuild, this));
		CurrentBuildActive = true;
	}
}

oP4ChangelistBuilder::ChangeInfo* oP4ChangelistBuilderImpl::GetNextBuild()
{
	oConcurrency::lock_guard<oConcurrency::shared_mutex> Lock(Mutex);
	oASSERT(!NextBuildInfos.empty(), "Popping an empty list");
	StartBuildMS = oTimerMS();
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
	LastBuildMS = (oTimerMS() - StartBuildMS);

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
		oStd::to_string(results.BuildName, CurrentBuild->CL);

	UpdateBuildProgress(CurrentBuild, "P4 Sync");

	// When re-syncing we blow away the old directory then force sync the directory again
	if (!oP4CleanSync(CurrentBuild->CL, P4Settings.Root.c_str(), CurrentTestSettings.ReSync ? BuildSettings.Solution.c_str() : nullptr))
	{
		oStd::lstring Error;
		oPrintf(Error, "P4 failed to sync with %s", oErrorGetLastString());
		oEmailAdminAndStop(EmailSettings, Error, CurrentBuild->CL, false);
	}

	oStd::path_string Path;
	oCleanPath(Path, LogRoot);
	oEnsureSeparator(Path);
	oStrAppendf(Path, results.BuildName.c_str());
	oEnsureSeparator(Path);
	oFileEnsureParentFolderExists(Path);

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
	return oInt(NextBuildInfos.size() + (uint)CurrentBuildActive);
}

void oP4ChangelistBuilderImpl::ReportWorking(oFUNCTION<void(const oP4ChangelistBuilder::ChangeInfo& _Change, int _RemainingMS, int _PercentageDone)> _Reporter) const
{
	oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(Mutex);

	// First calculate the time left for the current build
	int TimePastMS = oTimerMS() - StartBuildMS;
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

void oP4ChangelistBuilderImpl::ReportBuilt(oFUNCTION<void(const std::list<oP4ChangelistBuilder::ChangeInfo> & _Changes)> _Reporter) const
{
	// TODO: Take ownership over the iteration like in ReportWorking, then FinishedBuildInfos doesn't need to be in reversed order anymore
	oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(Mutex);
	_Reporter(FinishedBuildInfos);
}

void oP4ChangelistBuilderImpl::ReportLastSpecialBuild(oFUNCTION<void(const char* _pName, bool _Success, const char* _pLastSuccesful)> _Reporter) const
{
	oConcurrency::shared_lock<oConcurrency::shared_mutex> Lock(Mutex);
	if (FinishedDailyBuildInfos.size())
	{
		_Reporter(FinishedDailyBuildInfos.front().UserName.c_str(), FinishedDailyBuildInfos.front().Succeeded, LastSuccesfulDailyBuildInfo.UserName.c_str());
	}
}

bool oP4ChangelistBuilderImpl::ParseAutoBuildResults(int _CL, ChangeInfo* _pInfo)
{
	oStd::path_string PathToCL;
	oPrintf(PathToCL, "%s%d/index.html", LogRoot.c_str(), _CL);
	return ParseAutoBuildResults(PathToCL, _pInfo);
}

bool oP4ChangelistBuilderImpl::ParseAutoBuildResultsSpecialBuild(const char* _pName, ChangeInfo* _pInfo)
{
	oStd::path_string AbsolutePath;
	oPrintf(AbsolutePath, "%s%s/index.html", LogRoot.c_str(), _pName);
	return ParseAutoBuildResults(AbsolutePath, _pInfo);
}

bool oP4ChangelistBuilderImpl::ParseAutoBuildResults(const char* _pAbsolutePath, ChangeInfo* _pInfo)
{
	_pInfo->BuildTime = 0;
	_pInfo->TestTime = 0;
	_pInfo->PackTime = 0;

	std::shared_ptr<oStd::xml> XML = oXMLLoad(_pAbsolutePath);
	if (!XML)
		return false;

	auto TitleNode = XML->first_child(XML->root(), "title");
	if (!TitleNode)
		return false;

	oStd::sstring Title = XML->node_value(TitleNode);
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
