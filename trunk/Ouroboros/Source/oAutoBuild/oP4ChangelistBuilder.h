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
#ifndef oP4ChangelistBuilder_h
#define oP4ChangelistBuilder_h

#include <oBase/ini.h>

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
	virtual void ReportWorking(oFUNCTION<void(const oP4ChangelistBuilder::ChangeInfo& _Change, int _RemainingMS, int _PercentageDone)> _Reporter) const = 0;
	virtual void ReportBuilt(oFUNCTION<void(const std::list<oP4ChangelistBuilder::ChangeInfo> & _Changes)> _Reporter) const = 0;
	virtual void ReportLastSpecialBuild(oFUNCTION<void(const char* _pName, bool _Success, const char* _pLastSuccesful)> _Reporter) const  = 0;

	// This should be called at a regular interval when the process is doing nothing else.  This allows the
	// builder to forcibly terminate any child processes
	virtual void MainThreadYield(uint _Milleseconds) = 0;

	// Forcibly cancels all active builds.  This should be called prior to destruction
	virtual void Cancel() = 0;
};

oAPI bool oChangelistManagerCreate(const ouro::ini& _INI, const char* _LogRoot, int _ServerPort, oP4ChangelistBuilder** _ppChangelistManager);


#endif