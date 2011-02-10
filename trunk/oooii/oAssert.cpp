/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include "pch.h"
#include <oooii/oAssert.h>
#include <oooii/oDebugger.h>
#include <oooii/oHash.h>
#include <oooii/oPath.h>
#include <oooii/oProcess.h>
#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oThread.h>
#include <vector>

static oAssert::ACTION GetAction(oMsgBox::RESULT _Result)
{
	switch (_Result)
	{
		case oMsgBox::ABORT: return oAssert::ABORT;
		case oMsgBox::BREAK: return oAssert::BREAK;
		case oMsgBox::IGNORE_ALWAYS: return oAssert::IGNORE_ALWAYS;
		default: break;
	}

	return oAssert::IGNORE_ONCE;
}

struct oAssertContext : oSingleton<oAssertContext>
{
	oAssertContext()
		: LogFile(0)
	{
		oDebugger::Reference();
		oDebugger::print("--- oAssert initialized ---\n");
		*LogPath = 0;
	}

	~oAssertContext()
	{
		if (LogFile)
			fclose(LogFile);

		oDebugger::print("--- oAssert deinitialized ---\n");
		oDebugger::Release();
	}

	void vlogf(const char* _Format, va_list _Args)
	{
		if (LogFile)
		{
			char buf[2048];
			size_t n = vsprintf_s(buf, _Format, _Args);
			Log(buf, n);
		}
	}

	void Log(const char* _String, size_t _LengthHint = 0)
	{
		if (LogFile)
			fwrite(_String, _LengthHint ? _LengthHint : strlen(_String), 1, LogFile);
	}

	void SetDesc(const oAssert::DESC* _pDesc)
	{
		oAssert::DESC defaultDesc;

		if (!_pDesc)
			_pDesc = &defaultDesc;

		Desc = *_pDesc;

		if (Desc.LogFilePath)
		{
			#ifdef _DEBUG
				char tmp[_MAX_PATH];
				oCleanPath(tmp, Desc.LogFilePath);
				if (0 != _stricmp(LogPath, tmp))
				{
					if (LogFile)
					{
						fclose(LogFile);
						LogFile = 0;
					}

					if (0 != fopen_s(&LogFile, tmp, "wt"))
					{
						*LogPath = 0;
						oWARN("Failed to open log file \"%s\"", oSAFESTR(tmp));
						
						// make a copy of the path and attach it to the desc for future
						// GetDesc() calls.
						strcpy_s(LogPath, tmp);
						Desc.LogFilePath = LogPath;
					}
				}
			#else
				oWARNA("OOOii Debug Library logging to file is ignored in release builds.");
			#endif
		}

		else if (LogFile)
		{
			fclose(LogFile);
			LogFile = 0;
		}
	}

	inline const oAssert::DESC& GetDesc() const { return Desc; }
	void AddMessageFilter(int _MsgId) { oPushBackUnique(FilteredMessages, _MsgId); }
	void RemoveMessageFilter(int _MsgId) { oFindAndErase(FilteredMessages, _MsgId); }

	oAssert::ACTION VPrintMessage(const oAssert::ASSERTION* _pAssertion, const char* _Format, va_list _Args);

protected:
	FILE* LogFile;
	char LogPath[_MAX_PATH];
	oAssert::DESC Desc;
	typedef oArray<int, 256> array_t;
	array_t FilteredMessages;
};


static oAssert::ACTION ShowMsgBox(const oAssert::ASSERTION* _pAssertion, oMsgBox::TYPE _Type, const char* _String)
{
	static const char* DIALOG_BOX_TITLE = "OOOii Debug Library";

	char format[3 * 1024];
	*format = 0;
	char* end = format + sizeof(format);
	char* cur = format;
	cur += sprintf_s(format, "Debug %s!\n%s() %s(%u)\n\n", _Type == oMsgBox::WARN ? "Warning" : "Error", _pAssertion->Function, _pAssertion->Filename, _pAssertion->Line);
	strcpy_s(cur, std::distance(cur, end), _String);
	return GetAction(oMsgBox::printf(_Type, DIALOG_BOX_TITLE, "%s", format));
}

oAssert::ACTION OOOiiDefaultPrintMessage(const oAssert::ASSERTION* _pAssertion, const char* _Format, va_list _Args)
{
	oAssert::DESC desc;
	oAssert::GetDesc(&desc);

	// add prefixes to original message
	char msg[2048];
	{
		*msg = 0;
		char* end = msg + sizeof(msg);
		char* cur = msg;

		if (desc.PrefixFileLine)
			cur += sprintf_s(cur, std::distance(cur, end), "%s(%u) : ", _pAssertion->Filename, _pAssertion->Line);

		if (desc.PrefixThreadId)
			cur += sprintf_s(cur, std::distance(cur, end), "[%s.%u.%u] ", oGetHostname(), oProcess::GetCurrentProcessID(), oThread::GetCurrentThreadID());

		if (desc.PrefixMsgId)
			cur += sprintf_s(cur, std::distance(cur, end), "{0x%08x} ", _pAssertion->MsgId);

		strcpy_s(cur, std::distance(cur, end), _Format);
	}

	char final[2048];
	vsprintf_s(final, msg, _Args);

	// Always print any message to the debugger output
	oDebugger::print(final);

	oAssertContext::Singleton()->Log(final);

	// Output message
	oAssert::ACTION action = oAssert::IGNORE_ONCE;
	switch (_pAssertion->Type)
	{
		case oAssert::TYPE_TRACE:
			break;

		case oAssert::TYPE_WARNING:
			if (desc.EnableWarnings)
				action = ShowMsgBox(_pAssertion, oMsgBox::WARN, final);
			break;

		case oAssert::TYPE_ASSERT:
			action = ShowMsgBox(_pAssertion, oMsgBox::DEBUG, final);
			break;

		default: oASSUME(0);
	}

	return action;
}

oAssert::ACTION oAssertContext::VPrintMessage(const oAssert::ASSERTION* _pAssertion, const char* _Format, va_list _Args)
{
	if (oContains(FilteredMessages, _pAssertion->MsgId))
		return oAssert::IGNORE_ONCE;

	if (!Desc.VPrintMessage)
		return OOOiiDefaultPrintMessage(_pAssertion, _Format, _Args);

	return Desc.VPrintMessage(_pAssertion, _Format, _Args);
}

void oAssert::SetDesc(const DESC* _pDesc)
{
	oAssertContext::Singleton()->SetDesc(_pDesc);
}

void oAssert::GetDesc(DESC* _pDesc)
{
	*_pDesc = oAssertContext::Singleton()->GetDesc();
}

int oAssert::GetMsgId(const char* _Format)
{
	return (int)oHash_superfast(_Format, static_cast<unsigned int>(strlen(_Format)), 0);
}

oAssert::ACTION oAssert::VPrintMessage(const ASSERTION* _pAssertion, const char* _Format, va_list _Args)
{
	return oAssertContext::Singleton()->VPrintMessage(_pAssertion, _Format, _Args);
}

void oAssert::AddMessageFilter(int _MsgId)
{
	oAssertContext::Singleton()->AddMessageFilter(_MsgId);
}

void oAssert::RemoveMessageFilter(int _MsgId)
{
	oAssertContext::Singleton()->RemoveMessageFilter(_MsgId);
}
