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
#include "oMSBuild.h"

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oMSBUILD_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oMSBUILD_SETTINGS)
	oRTTI_COMPOUND_VERSION(oMSBUILD_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oMSBUILD_SETTINGS)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, TimeoutSeconds, oRTTI_OF(uint), "TimeoutSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, ToolPath, oRTTI_OF(ostd_path_string), "ToolPath", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, Solution, oRTTI_OF(ostd_path_string), "Solution", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, CleanAlways, oRTTI_OF(bool), "CleanAlways", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, Configurations, oRTTI_OF(std_vector_ostd_sstring), "Configurations", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, Platforms, oRTTI_OF(std_vector_ostd_sstring), "Platforms", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, ThreadsPerBuild, oRTTI_OF(int), "ThreadsPerBuild", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oMSBUILD_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oMSBUILD_SETTINGS)

static const char* oMSBUILD_ERROR_PATTERNS[2] = {": error ", ": fatal error"};
static const char* oMSBUILD_WARNING_PATTERN = ": warning ";

static bool MSBuild(const oMSBUILD_SETTINGS& _Settings, const char* _pCommand, oFUNCTION<bool(const oStd::uri_string& _CommandName, const char* _pLog)> _CommandLogger)
{
	oStd::lstring ToolName;
	oPrintf(ToolName, "%s /nodereuse:false /maxcpucount", _Settings.ToolPath);
	if(_Settings.ThreadsPerBuild > 0)
		oStrAppendf(ToolName, ":%d", _Settings.ThreadsPerBuild);

	oStd::path_string CommandLine;
	int ConfigHead = oPrintf(CommandLine, "%s %s /p:Configuration=", ToolName, _Settings.Solution);

	o_msbuild_stdout_t StdOutDrain;
	std::unordered_map<oStd::uri_string, oRef<threadsafe oProcess>, oStdHash<oStd::uri_string>, oStd::equal_to<oStd::uri_string>> BuildProcesses;
	oFOR(auto& Config, _Settings.Configurations)
	{
		int PlatformHead = ConfigHead + oPrintf(CommandLine.c_str() + ConfigHead, CommandLine.capacity() - ConfigHead, "%s /p:Platform=", Config);

		oFOR(auto& Platform, _Settings.Platforms)
		{
			oPrintf(CommandLine.c_str() + PlatformHead, CommandLine.capacity() - PlatformHead, "%s %s", Platform, _pCommand);

			oProcess::DESC ProcessDesc;
			ProcessDesc.CommandLine = CommandLine;
			ProcessDesc.StartSuspended = false;
			ProcessDesc.StdHandleBufferSize = StdOutDrain.capacity() - 1;
			ProcessDesc.ShowWindow = false;

			oRef<threadsafe oProcess> Process;
			if(!oProcessCreate(ProcessDesc, &Process))
			{
				return false;
			}

			oStd::uri_string Name;
			oPrintf(Name, "%s_%s", Config, Platform);
			if( BuildProcesses.end() != BuildProcesses.find(Name) )
				return oErrorSetLast(std::errc::invalid_argument, "Duplicate build requested %s", Name);
			BuildProcesses[Name] = Process;
		}
	}

	while(!BuildProcesses.empty())
	{
		for(auto BuildProcess = BuildProcesses.begin(); BuildProcess != BuildProcesses.end();)
		{
			auto& Process = BuildProcess->second;

			bool Finished = Process->Wait(5);

			size_t SuccessRead = Process->ReadFromStdout(StdOutDrain.c_str(), StdOutDrain.capacity() - 1);
			StdOutDrain[SuccessRead] = 0;
			if( SuccessRead > 0 && !_CommandLogger(BuildProcess->first, StdOutDrain) )
			{
				oFOR(auto ProcessToTerminate, BuildProcesses )
				{
					ProcessToTerminate.second->Kill(0);
				}
				return oErrorSetLast(std::errc::operation_canceled);
			}

			if(Finished)
			{
				BuildProcess = BuildProcesses.erase(BuildProcess);
			}
			else
				++BuildProcess;
		}
	}

	return true;
}

struct LogContext
{
	LogContext()
		: LastParsed(0)
	{
		Log.reserve(oKB(64));
	}

	bool ContainsError(const char* _pLog)
	{
		Log.append(_pLog);
		return ContainsError();
	}
	bool ContainsError()
	{
		const char* pCurrentLog = Log.c_str();
		while(true)
		{
			const char* pLine = pCurrentLog + LastParsed;
			const char* pNewEnd = strchr(pLine, '\n');
			if(!pNewEnd)
				break;

			for(size_t i = 0; i < oCOUNTOF(oMSBUILD_ERROR_PATTERNS); ++i)
			{
				if( nullptr != strstr(pLine, oMSBUILD_ERROR_PATTERNS[i]))
					return true;
			}
			

			LastParsed += (pNewEnd - pLine) + 1;
		}
		return false;
	}
	const std::string& GetLog() const { return Log; }
private:
	size_t LastParsed;
	std::string Log;
};

bool oMSBuildAndLog(const oMSBUILD_SETTINGS& _Settings, const char* _LogFolder, const oConcurrency::event& _CancelEvent, oMSBuildResults* _pResults)
{
	std::unordered_map<oStd::uri_string, LogContext, oStdHash<oStd::uri_string>, oStd::equal_to<oStd::uri_string>> BuildLogs;

	_pResults->CleanSucceeded = true;
	_pResults->CleanTimeSeconds = 0.0f;
	if (_Settings.CleanAlways)
	{
		float StartCleanTimeMS = oTimerMSF();
		_pResults->CleanSucceeded = MSBuild(_Settings, "/t:Clean", [&](const oStd::uri_string& _CommandName, const char* _pLog)->bool
		{
			return true;
		});
		_pResults->CleanTimeSeconds = (oTimerMSF() - StartCleanTimeMS ) / 1000.0f;

		if (!_pResults->CleanSucceeded)
			return oErrorSetLast(std::errc::invalid_argument);
	}

	if (_CancelEvent.is_set())
		return false;

	uint TimeoutMS = _Settings.TimeoutSeconds * 1000;
	oScopedPartialTimeout Timer(&TimeoutMS);

	float StartBuildTimeMS = oTimerMSF();
	_pResults->BuildTimeSeconds = 0.0f;
	_pResults->BuildSucceeded = MSBuild(_Settings, "", [&](const oStd::uri_string& _CommandName, const char* _pLog)->bool
	{
		if (_CancelEvent.is_set() || 0 == TimeoutMS)
			return false;

		Timer.UpdateTimeout();
		return !BuildLogs[_CommandName].ContainsError(_pLog);
	});
	_pResults->BuildTimeSeconds = (oTimerMSF() - StartBuildTimeMS ) / 1000.0f;
	_pResults->BuildTimedOut = !_pResults->BuildSucceeded && (0 == TimeoutMS);

	if(_CancelEvent.is_set())
		return false;

	if (_pResults->BuildTimedOut)
		return oErrorSetLast(std::errc::invalid_argument);

	_pResults->SavingLogfilesSucceeded = true;
	oFOR(auto& BuildLog, BuildLogs)
	{
		// Save out the file
		oStd::path_string FilePath = _LogFolder;
		oEnsureSeparator(FilePath);
		oStrAppendf(FilePath, "%s.txt", BuildLog.first.c_str());
		auto& Log = BuildLog.second.GetLog();
		if (!oFileSave(FilePath, &Log[0], Log.size(), true))
			_pResults->SavingLogfilesSucceeded &= false;

		_pResults->BuildLogfiles.push_back(FilePath);
	}

	if (!_pResults->BuildSucceeded)
		return oErrorSetLast(std::errc::protocol_error, "broke the build");

	return true;
}

bool oMSBuildParseLogfile(oStd::path_string _Logfile, bool _IncludeWarnings, oFUNCTION<bool(o_msbuild_stdout_t _WarningOrError)> _Output)
{
	oRef<oBuffer> Buffer;
	if (!oBufferLoad(_Logfile, &Buffer, true))
		return false;

	const char* pLog = (const char*)Buffer->GetData();
	while(*pLog)
	{
		const char* pEnd = strchr(pLog, '\n');

		if(!pEnd)
			break;

		size_t LineLength = pEnd - pLog;
		bool WarningOrError = false;
		for(size_t i = 0; i < oCOUNTOF(oMSBUILD_ERROR_PATTERNS); ++i)
		{
			if (nullptr != oStrStr(pLog, oMSBUILD_ERROR_PATTERNS[i], LineLength))
				WarningOrError = true;
		}

		if (_IncludeWarnings)
			WarningOrError |= (nullptr != oStrStr(pLog, oMSBUILD_WARNING_PATTERN, LineLength));

		if (WarningOrError)
		{
			o_msbuild_stdout_t ErrorLine;
			oStrncpy(ErrorLine, pLog, LineLength);
			if (!_Output(ErrorLine))
				break;
		}

		pLog = pEnd + 1;
	}
	return true;
}