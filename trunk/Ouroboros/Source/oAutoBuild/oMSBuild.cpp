/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oBasis/oScopedPartialTimeout.h>

using namespace ouro;

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oMSBUILD_SETTINGS)
	oRTTI_COMPOUND_ABSTRACT(oMSBUILD_SETTINGS)
	oRTTI_COMPOUND_VERSION(oMSBUILD_SETTINGS, 0,1,0,0)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oMSBUILD_SETTINGS)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, TimeoutSeconds, oRTTI_OF(uint), "TimeoutSeconds", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, ToolPath, oRTTI_OF(ouro_path_string), "ToolPath", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, Solution, oRTTI_OF(ouro_path_string), "Solution", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, CleanAlways, oRTTI_OF(bool), "CleanAlways", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, Configurations, oRTTI_OF(std_vector_ouro_sstring), "Configurations", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, Platforms, oRTTI_OF(std_vector_ouro_sstring), "Platforms", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oMSBUILD_SETTINGS, ThreadsPerBuild, oRTTI_OF(int), "ThreadsPerBuild", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oMSBUILD_SETTINGS)
oRTTI_COMPOUND_END_DESCRIPTION(oMSBUILD_SETTINGS)

static const char* oStrStr( const char* _pStr, const char* _pSubStr, size_t _MaxCharCount /*= ouro::invalid*/ )
{
	if(ouro::invalid == _MaxCharCount)
		return strstr(_pStr, _pSubStr);

	size_t SearchLen = strlen(_pSubStr);
	size_t SearchHead = 0;
	for(size_t i = 0; i < _MaxCharCount; ++i)
	{
		if(_pStr[i] == _pSubStr[SearchHead])
		{
			if( ++SearchHead == SearchLen)
			{
				// Found a match
				return _pStr + ( i - SearchLen );
			}
		}
		else
			SearchHead = 0;
	}
	return nullptr;
}

static const char* oMSBUILD_ERROR_PATTERNS[2] = {": error ", ": fatal error"};
static const char* oMSBUILD_WARNING_PATTERN = ": warning ";

static bool MSBuild(const oMSBUILD_SETTINGS& _Settings, const char* _pCommand, std::function<bool(const uri_string& _CommandName, const char* _pLog)> _CommandLogger)
{
	lstring ToolName;
	snprintf(ToolName, "%s /nodereuse:false /maxcpucount", _Settings.ToolPath);
	if(_Settings.ThreadsPerBuild > 0)
		sncatf(ToolName, ":%d", _Settings.ThreadsPerBuild);

	path_string CommandLine;
	int ConfigHead = snprintf(CommandLine, "%s %s /p:Configuration=", ToolName, _Settings.Solution);

	o_msbuild_stdout_t StdOutDrain;
	std::unordered_map<uri_string, std::shared_ptr<ouro::process>, oStdHash<uri_string>, ouro::same<uri_string>> BuildProcesses;
	for (auto& Config : _Settings.Configurations)
	{
		int PlatformHead = ConfigHead + snprintf(CommandLine.c_str() + ConfigHead, CommandLine.capacity() - ConfigHead, "%s /p:Platform=", Config);

		for (auto& Platform : _Settings.Platforms)
		{
			snprintf(CommandLine.c_str() + PlatformHead, CommandLine.capacity() - PlatformHead, "%s %s", Platform, _pCommand);

			ouro::process::info pi;
			pi.command_line = CommandLine;
			pi.stdout_buffer_size = StdOutDrain.capacity() - 1;
			pi.show = ouro::process::hide;

			std::shared_ptr<ouro::process> Process = ouro::process::make(pi);
			uri_string Name;
			snprintf(Name, "%s_%s", Config, Platform);
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

			bool Finished = Process->wait_for(std::chrono::seconds(5));

			size_t SuccessRead = Process->from_stdout(StdOutDrain.c_str(), StdOutDrain.capacity() - 1);
			StdOutDrain[SuccessRead] = 0;
			if( SuccessRead > 0 && !_CommandLogger(BuildProcess->first, StdOutDrain) )
			{
				for (auto ProcessToTerminate : BuildProcesses )
				{
					ProcessToTerminate.second->kill(0);
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

bool oMSBuildAndLog(const oMSBUILD_SETTINGS& _Settings, const char* _LogFolder, const ouro::event& _CancelEvent, oMSBuildResults* _pResults)
{
	std::unordered_map<uri_string, LogContext, oStdHash<uri_string>, ouro::same<uri_string>> BuildLogs;

	_pResults->CleanSucceeded = true;
	_pResults->CleanTimeSeconds = 0.0f;
	if (_Settings.CleanAlways)
	{
		float StartCleanTimeMS = ouro::timer::nowmsf();
		_pResults->CleanSucceeded = MSBuild(_Settings, "/t:Clean", [&](const uri_string& _CommandName, const char* _pLog)->bool
		{
			return true;
		});
		_pResults->CleanTimeSeconds = (ouro::timer::nowmsf() - StartCleanTimeMS ) / 1000.0f;

		if (!_pResults->CleanSucceeded)
			return oErrorSetLast(std::errc::invalid_argument);
	}

	if (_CancelEvent.is_set())
		return false;

	uint TimeoutMS = _Settings.TimeoutSeconds * 1000;
	oScopedPartialTimeout Timer(&TimeoutMS);

	float StartBuildTimeMS = ouro::timer::nowmsf();
	_pResults->BuildTimeSeconds = 0.0f;
	_pResults->BuildSucceeded = MSBuild(_Settings, "", [&](const uri_string& _CommandName, const char* _pLog)->bool
	{
		if (_CancelEvent.is_set() || 0 == TimeoutMS)
			return false;

		Timer.UpdateTimeout();
		return !BuildLogs[_CommandName].ContainsError(_pLog);
	});
	_pResults->BuildTimeSeconds = (ouro::timer::nowmsf() - StartBuildTimeMS ) / 1000.0f;
	_pResults->BuildTimedOut = !_pResults->BuildSucceeded && (0 == TimeoutMS);

	if(_CancelEvent.is_set())
		return false;

	if (_pResults->BuildTimedOut)
		return oErrorSetLast(std::errc::invalid_argument);

	_pResults->SavingLogfilesSucceeded = true;
	for (auto& BuildLog : BuildLogs)
	{
		// Save out the file
		path FilePath = _LogFolder;
		sstring Filename;
		snprintf(Filename, "%s.txt", BuildLog.first.c_str());
		FilePath /= Filename;
		auto& Log = BuildLog.second.GetLog();
		
		try { ouro::filesystem::save(FilePath, &Log[0], Log.size(), ouro::filesystem::save_option::text_write); }
		catch (std::exception&) { _pResults->SavingLogfilesSucceeded &= false; }

		_pResults->BuildLogfiles.push_back(FilePath);
	}

	if (!_pResults->BuildSucceeded)
		return oErrorSetLast(std::errc::protocol_error, "broke the build");

	return true;
}

bool oMSBuildParseLogfile(path_string _Logfile, bool _IncludeWarnings, std::function<bool(o_msbuild_stdout_t _WarningOrError)> _Output)
{
	scoped_allocation Buffer;
	try { Buffer = ouro::filesystem::load(ouro::path(_Logfile), ouro::filesystem::load_option::text_read); }
	catch (std::exception&)
	{
		return false;
	}

	const char* pLog = (const char*)Buffer;
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
			strncpy(ErrorLine, pLog, LineLength);
			if (!_Output(ErrorLine))
				break;
		}

		pLog = pEnd + 1;
	}
	return true;
}