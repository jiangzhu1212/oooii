// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma  once
#ifndef oP4ChangelistBuilder_h
#define oP4ChangelistBuilder_h

#include <oString/ini.h>

static const char* oAUTO_BUILD_SPECIAL_PREFIX = "Daily";
static const char* oBUILD_TOOL_FAIL_TITLE = " Fail";
static const char* oBUILD_TOOL_PASS_TITLE = " Pass";

// Builds p4 changelists 
interface oP4ChangelistBuilder : oInterface
{
	// All information describing a built changelist
	struct ChangeInfo
	{
		bool IsDaily;
		bool Succeeded;
		int CL;
		ouro::sstring UserName;
		ouro::sstring Date;
		ouro::sstring Stage;
		int BuildTime;
		int TestTime;
		int PackTime;
	};

	// Checks if there is a new check-in or if a new daily build needs to be built
	virtual void TryNextBuild(int _DailyBuildHour) = 0;

	// Number of changelists currently in the queue
	virtual int GetCount() const = 0;

	// Various reporting calls to determine the status of the builds
	virtual void ReportWorking(std::function<void(const oP4ChangelistBuilder::ChangeInfo& _Change, int _RemainingMS, int _PercentageDone)> _Reporter) const = 0;
	virtual void ReportBuilt(std::function<void(const std::list<oP4ChangelistBuilder::ChangeInfo> & _Changes)> _Reporter) const = 0;
	virtual void ReportLastSpecialBuild(std::function<void(const char* _pName, bool _Success, const char* _pLastSuccesful)> _Reporter) const  = 0;

	// This should be called at a regular interval when the process is doing nothing else.  This allows the
	// builder to forcibly terminate any child processes
	virtual void MainThreadYield(uint _Milleseconds) = 0;

	// Forcibly cancels all active builds.  This should be called prior to destruction
	virtual void Cancel() = 0;
};

oAPI bool oChangelistManagerCreate(const ouro::ini& _INI, const char* _LogRoot, int _ServerPort, oP4ChangelistBuilder** _ppChangelistManager);


#endif