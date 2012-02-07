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
#include <oPlatform/oReporting.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oArray.h>
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oSystem.h>
#include "oDbgHelp.h"
#include "oFileInternal.h"

const char* oAsString(const oASSERT_TYPE& _Type)
{
	switch (_Type)
	{
		case oASSERT_TRACE: return "Trace";
		case oASSERT_WARNING: return "Warning";
		case oASSERT_ASSERTION: return "Error";
		default: oASSERT_NOEXECUTION;
	}
}

struct oReportingContext : oProcessSingleton<oReportingContext>
{
	oReportingContext();

	void SetDesc(const oREPORTING_DESC& _Desc);
	inline void GetDesc(oREPORTING_DESC* _pDesc) { *_pDesc = Desc; }
	bool PushReporter(oReportingVPrint _Reporter);
	oReportingVPrint PopReporter();
	inline void AddFilter(size_t _AssertionID) { oPushBackUnique(FilteredMessages, _AssertionID); }
	inline void RemoveFilter(size_t _AssertionID) { oFindAndErase(FilteredMessages, _AssertionID); }
	oASSERT_ACTION VPrint(const oASSERTION& _Assertion, const char* _Format, va_list _Args);
	static oASSERT_ACTION DefaultVPrint(const oASSERTION& _Assertion, threadsafe oFileWriter* _pLogFile, const char* _Format, va_list _Args);
	static const oGUID GUID;

protected:
	oRef<oDbgHelp> DbgHelp;
	oRef<threadsafe oFileWriter> LogFile;
	oStringPath LogPath;
	oREPORTING_DESC Desc;
	typedef oArray<size_t, 256> array_t;
	array_t FilteredMessages;
	oArray<oReportingVPrint, 8> VPrintStack;
	oRecursiveMutex Mutex;
};

// {338D483B-7793-4BE1-90B1-4BB986B3EC2D}
const oGUID oReportingContext::GUID = { 0x338d483b, 0x7793, 0x4be1, { 0x90, 0xb1, 0x4b, 0xb9, 0x86, 0xb3, 0xec, 0x2d } };

oReportingContext::oReportingContext()
	: DbgHelp(oDbgHelp::Singleton())
{
	PushReporter(DefaultVPrint);
}

struct oLogWriter : oFileWriter
{
	// @oooii-tony: Temporary - Flush operations around IOCP assume files are 
	// closed out, but here in logging land, we keep the log file open. So 
	// implement a stop-gap while we figure out something smarter to do and retain
	// the oFileWriter interface broadcast to the user-callback for reporting.

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oRefCount RefCount;
	oHFILE hFile;
	oInitOnce<oStringPath> Path;

	oLogWriter(const char* _Path, bool* _pSuccess)
		: hFile(nullptr)
		, Path(_Path)
	{
		*_pSuccess = oFileOpen(_Path, oFILE_OPEN_BIN_WRITE, &hFile);
	}

	~oLogWriter()
	{
		if (hFile)
			oFileClose(hFile);
	}

	void DispatchWrite(const void* _pData, const oFileRange& _Range, callback_t _Callback) threadsafe override { oASSERT(false, "Don't call this"); }
	bool Write(const void* _pData, const oFileRange& _Range) threadsafe override
	{
		if (_Range.Offset == ~0ull)
		{
			if (!oFileSeek(hFile, 0, oSEEK_END))
				return false; // pass through error
		}
		
		else if (!oFileSeek(hFile, _Range.Offset))
			return false; // pass through error

		return _Range.Size == oFileWrite(hFile, _pData, _Range.Size); // pass through error
	}

	void GetDesc(oFILE_DESC* _pDesc) threadsafe override
	{
		oVERIFY(oFileGetDesc(*Path, _pDesc));
	}

	const char* GetPath() const threadsafe override { return *Path; }
};

static bool oLogFileWriterCreate(const char* _Path, threadsafe oFileWriter** _ppFileWriter)
{
	bool success = false;
	oCONSTRUCT(_ppFileWriter, oLogWriter(_Path, &success));
	return success;
}

void oReportingContext::SetDesc(const oREPORTING_DESC& _Desc)
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);

	Desc = _Desc;
	if (Desc.LogFilePath)
	{
		oStringPath tmp;
		oCleanPath(tmp.c_str(), tmp.capacity(), Desc.LogFilePath);
		if (_stricmp(LogPath, tmp))
		{
			LogFile = nullptr;
			if (!oLogFileWriterCreate(tmp, &LogFile))
			{
				LogPath.clear();
				oWARN("Failed to open log file \"%s\"\n%s: %s", tmp.c_str(), oAsString(oErrorGetLast()), oErrorGetLastString());
			}
		}

		// make a copy of the path and attach it to the desc for future
		// GetDesc() calls and reassign pointer.
		LogPath = tmp;
		Desc.LogFilePath = LogPath.c_str();
	}

	else 
		LogFile = nullptr;
}

bool oReportingContext::PushReporter(oReportingVPrint _Reporter)
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);

	if (VPrintStack.size() >= VPrintStack.capacity())
		return false;
	VPrintStack.push_back(_Reporter);
	return true;
}

oReportingVPrint oReportingContext::PopReporter()
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);

	oReportingVPrint fn = nullptr;
	if (!VPrintStack.empty())
	{
		fn = VPrintStack.back();
		VPrintStack.pop_back();
	}
	return fn;
}

oASSERT_ACTION oReportingContext::VPrint(const oASSERTION& _Assertion, const char* _Format, va_list _Args)
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);

	if (!oContains(FilteredMessages, _Assertion.ID) && !VPrintStack.empty())
	{
		oReportingVPrint VPrintMessage = VPrintStack.back();
		return VPrintMessage(_Assertion, LogFile, _Format, _Args);
	}

	return oASSERT_IGNORE_ONCE;
}

void oReportingReference()
{
	// If this crashes due to a null reference, it means there's a bad teardown
	// order in client code. Don't fix here, fix the ordering.
	intrusive_ptr_add_ref(oReportingContext::Singleton());
}

void oReportingRelease()
{
	// If this crashes due to a null reference, it means there's a bad teardown
	// order in client code. Don't fix here, fix the ordering.
	intrusive_ptr_release(oReportingContext::Singleton());
}

void oReportingSetDesc(const oREPORTING_DESC& _Desc)
{
	oReportingContext::Singleton()->SetDesc(_Desc);
}

void oReportingGetDesc(oREPORTING_DESC* _pDesc)
{
	oReportingContext::Singleton()->GetDesc(_pDesc);
}

bool oReportingPushReporter(oReportingVPrint _Reporter)
{
	return oReportingContext::Singleton()->PushReporter(_Reporter);
}

oReportingVPrint oReportingPopReporter()
{
	return oReportingContext::Singleton()->PopReporter();
}

void oReportingAddFilter(size_t _AssertionID)
{
	oReportingContext::Singleton()->AddFilter(_AssertionID);
}

void oReportingRemoveFilter(size_t _AssertionID)
{
	oReportingContext::Singleton()->RemoveFilter(_AssertionID);
}

oASSERT_ACTION oAssertVPrintf(const oASSERTION& _Assertion, const char* _Format, va_list _Args)
{
	return oReportingContext::Singleton()->VPrint(_Assertion, _Format, _Args);
}

static oASSERT_ACTION GetAction(oMSGBOX_RESULT _Result)
{
	switch (_Result)
	{
	case oMSGBOX_ABORT: return oASSERT_ABORT;
	case oMSGBOX_BREAK: return oASSERT_BREAK;
	case oMSGBOX_IGNORE: return oASSERT_IGNORE_ALWAYS;
	default: break;
	}

	return oASSERT_IGNORE_ONCE;
}

static oASSERT_ACTION ShowMsgBox(const oASSERTION& _Assertion, oMSGBOX_TYPE _Type, const char* _String)
{
#ifdef _DEBUG
	static const char* MESSAGE_PREFIX = "Debug %s!\n\n";
	static const char* DIALOG_BOX_TITLE = "OOOii Debug Library";
#else
	static const char* MESSAGE_PREFIX = "Release %s!\n\n";
	static const char* DIALOG_BOX_TITLE = "OOOii Release Library";
#endif

	char format[32 * 1024];
	*format = 0;
	char* end = format + sizeof(format);
	char* cur = format;
	cur += sprintf_s(format, MESSAGE_PREFIX, _Type == oMSGBOX_WARN ? "Warning" : "Error");
	strcpy_s(cur, std::distance(cur, end), _String);

	char path[_MAX_PATH];
	oSystemGetPath(path, oSYSPATH_APP_FULL);
	char title[1024];
	sprintf_s(title, "%s (%s)", DIALOG_BOX_TITLE, path);

	oMSGBOX_DESC mb;
	mb.Type = _Type;
	mb.Title = title;
	return GetAction(oMsgBox(mb, "%s", format));
}

#define oACCUM_PRINTF(_Format, ...) do \
	{	res = _snprintf_s(_StrDestination + len, _SizeofStrDestination - len - 1, _TRUNCATE, _Format, ## __VA_ARGS__); \
		if (res == -1) goto TRUNCATION; \
		len += res; \
	} while(false)

#define oACCUM_VPRINTF(_Format, _Args) do \
	{	res = _vsnprintf_s(_StrDestination + len, _SizeofStrDestination - len - 1, _TRUNCATE, _Format, _Args); \
		if (res == -1) goto TRUNCATION; \
		len += res; \
	} while(false)

static void PrintCallStackToString(char* _StrDestination, size_t _SizeofStrDestination, bool _FilterStdBind)
{
	int res = 0;
	size_t len = 0;

	size_t offset = 6; // Start offset from where assert occurred, skipping any debug handling code
	size_t nSymbols = 0;
	*_StrDestination = 0;
	unsigned long long address = 0;
	bool IsStdBind = false;
	while (oDebuggerGetCallstack(&address, 1, offset++))
	{
		if (nSymbols++ == 0) // if we have a callstack, label it
		{
			res = _snprintf_s(_StrDestination, _SizeofStrDestination, _TRUNCATE, "\nCall Stack:\n");
			if (res == -1) goto TRUNCATION;
			len += res;
		}

		bool WasStdBind = IsStdBind;
		res = oDebuggerSymbolSPrintf(&_StrDestination[len], _SizeofStrDestination - len - 1, address, "", &IsStdBind);
		if (res == -1) goto TRUNCATION;
		len += res;

		if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
			offset += 5;
	}

	return;

	TRUNCATION:
		static const char* kStackTooLargeMessage = "\n... truncated ...";
		size_t TLMLength = strlen(kStackTooLargeMessage);
		sprintf_s(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
}

char* FormatAssertMessage(char* _StrDestination, size_t _SizeofStrDestination, const oREPORTING_DESC& _Desc, const oASSERTION& _Assertion, const char* _Format, va_list _Args)
{
	int res = 0;
	size_t len = 0;
	if (_Desc.PrefixFileLine)
	{
		#ifdef oCLICKABLE_OUTPUT_REQUIRES_SPACE_COLON
			static const char* kClickableFileLineFormat = "%s(%u) : ";
		#else
			static const char* kClickableFileLineFormat = "%s(%u): ";
		#endif
		oACCUM_PRINTF(kClickableFileLineFormat, _Assertion.Filename, _Assertion.Line);
	}

	if (_Desc.PrefixMsgType)
		oACCUM_PRINTF("%s ", oAsString(_Assertion.Type));

	if (_Desc.PrefixThreadId)
	{
		char syspath[_MAX_PATH];
		oACCUM_PRINTF("%s ", oSystemGetPath(syspath, oSYSPATH_EXECUTION));
	}

	if (_Desc.PrefixMsgId)
		oACCUM_PRINTF("{0x%08x} ", _Assertion.ID);

	oACCUM_VPRINTF(_Format, _Args);
	return _StrDestination + len;

TRUNCATION:
	static const char* kStackTooLargeMessage = "\n... truncated ...";
	size_t TLMLength = strlen(kStackTooLargeMessage);
	sprintf_s(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
	return _StrDestination + _SizeofStrDestination;
}

template<size_t size> inline char* FormatAssertMessage(char (&_StrDestination)[size], const oREPORTING_DESC& _Desc, const oASSERTION& _Assertion, const char* _Format, va_list _Args) { return FormatAssertMessage(_StrDestination, size, _Desc, _Assertion, _Format, _Args); }

oASSERT_ACTION oReportingContext::DefaultVPrint(const oASSERTION& _Assertion, threadsafe oFileWriter* _pLogFile, const char* _Format, va_list _Args)
{
	oREPORTING_DESC desc;
	oReportingGetDesc(&desc);

	bool addCallStack = desc.PrintCallstack && (_Assertion.Type == oASSERT_ASSERTION);

	// add prefixes to original message
	char msg[oKB(8)];
	char* cur = FormatAssertMessage(msg, desc, _Assertion, _Format, _Args);
	char* end = msg + sizeof(msg);

	if (addCallStack)
		PrintCallStackToString(cur, std::distance(cur, end), true);

	// Always print any message to the debugger output
	oDebuggerPrint(msg);

	// And to log file

	if (_pLogFile)
	{
		oFileRange r;
		r.Offset = oFILE_APPEND;
		r.Size = strlen(msg);
		_pLogFile->Write(msg, r);
	}

	// Output message
	oASSERT_ACTION action = _Assertion.DefaultResponse;
	switch (_Assertion.Type)
	{
		case oASSERT_TRACE:
			break;

		case oASSERT_WARNING:
			if (desc.PromptWarnings)
				action = ShowMsgBox(_Assertion, oMSGBOX_WARN, msg);
			break;

		case oASSERT_ASSERTION:
			if (desc.PromptAsserts)
				action = ShowMsgBox(_Assertion, oMSGBOX_DEBUG, msg);
			break;

		default: oASSERT_NOEXECUTION;
	}

	return action;
}
