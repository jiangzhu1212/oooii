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
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oStandards.h>
#include "SoftLink/oWinDbgHelp.h"
#include "oCRTLeakTracker.h"
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

static int oWinWriteDumpFile_Helper(MINIDUMP_TYPE _Type, HANDLE _hFile, bool* _pSuccess, EXCEPTION_POINTERS* _pExceptionPointers)
{
	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	ExInfo.ThreadId = GetCurrentThreadId();
	ExInfo.ExceptionPointers = _pExceptionPointers;
	ExInfo.ClientPointers = TRUE; // true because we're in THIS process, this might need to change if this is called from another process.
	*_pSuccess = !!oWinDbgHelp::Singleton()->MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), _hFile, _Type, _pExceptionPointers ? &ExInfo : nullptr, nullptr, nullptr);
	return EXCEPTION_EXECUTE_HANDLER;
}

static bool oWinWriteDumpFile(MINIDUMP_TYPE _Type, const char* _Path, EXCEPTION_POINTERS* _pExceptionPointers)
{
	// Use most-direct APIs for this so there's less chance another crash/assert
	// can occur.
	oFileEnsureParentFolderExists(_Path);
	HANDLE hFile = CreateFileA(_Path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to open %s for write.", oSAFESTRN(_Path));

	bool success = false; 
	if (_pExceptionPointers)
		oWinWriteDumpFile_Helper(_Type, hFile, &success, _pExceptionPointers);
	else
	{
		// If you're here, especially from a dump file, it's because the file was 
		// dumped outside an exception handler. In order to get stack info, we need
		// to cause an exception. See Remarks section:
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms680360(v=vs.85).aspx

		// So from here, somewhere up the stack should be the line of code that 
		// triggered execution of this. Happy debugging!
		const static DWORD FORCE_EXCEPTION_FOR_CALLSTACK_INFO = 0x1337c0de;
		__try { RaiseException(FORCE_EXCEPTION_FOR_CALLSTACK_INFO, 0, 0, nullptr); }
		__except(oWinWriteDumpFile_Helper(_Type, hFile, &success, GetExceptionInformation())) {}
	}

	CloseHandle(hFile);
	if (!success)
		return oErrorSetLast(oERROR_GENERIC, "Failed to write dump file %s", oSAFESTRN(_Path));
	return success;
}

//static bool oWinWriteDumpFile(EXCEPTION_POINTERS* _pExceptionPointers)
//{
//	oStringPath DumpPath;
//	oSystemGetPath(DumpPath, oSYSPATH_APP_FULL);
//	oTrimFileExtension(DumpPath);
//	oStrAppendf(DumpPath, ".dmp");
//	return oWinWriteDumpFile(DumpPath, _pExceptionPointers);
//}

void ReportErrorAndExit()
{
	oASSERT(false, "std::terminate Called");
}

struct oReportingContext : oProcessSingleton<oReportingContext>
{
	oReportingContext();
	~oReportingContext();

	void SetDesc(const oREPORTING_DESC& _Desc);
	inline void GetDesc(oREPORTING_DESC* _pDesc) { *_pDesc = Desc; }
	bool PushReporter(oReportingVPrint _Reporter);
	oReportingVPrint PopReporter();
	inline void AddFilter(size_t _AssertionID) { oPushBackUnique(FilteredMessages, _AssertionID); }
	inline void RemoveFilter(size_t _AssertionID) { oFindAndErase(FilteredMessages, _AssertionID); }
	oASSERT_ACTION VPrint(const oASSERTION& _Assertion, const char* _Format, va_list _Args);
	static oASSERT_ACTION DefaultVPrint(const oASSERTION& _Assertion, threadsafe oStreamWriter* _pLogFile, const char* _Format, va_list _Args);
	static const oGUID GUID;
	void EnableErrorDialogBoxes(bool _bValue);
	bool AreDialogBoxesEnabled() const { return bDialogBoxesEnabled; }
	static oReportingContext* Singleton()
	{
		// Construction of a singleton currently goes ProcessHeap->CRTLeakTracker->ReportingContext,
		// so if this is the first time through, then we'll get a circular reference to an uninit'ed Singleton
		// So the first time, ensure we construct everything below this before really going in.
		static bool once = false;
		if (!once)
		{
			oProcessHeapEnsureRunning();
			once = true;
		}

		return oProcessSingleton<oReportingContext>::Singleton();
	}

	void DumpAndTerminate(EXCEPTION_POINTERS* _pExceptionPtrs, const char* _pUserErrorMessage)
	{
		oStringS DumpStamp = oGetModuleFileStampString();

		bool Mini = false;
		bool Full = false;

		oStringPath DumpPath;
		if (MiniDumpBase)
		{
			oPrintf(DumpPath, "%s%s.dmp", MiniDumpBase, DumpStamp.c_str());
			Mini = oWinWriteDumpFile(MiniDumpNormal, DumpPath, _pExceptionPtrs);
		}

		if (FullDumpBase)
		{
			oPrintf(DumpPath, "%s%s.dmp", FullDumpBase, DumpStamp.c_str());
			Full = oWinWriteDumpFile(MiniDumpWithFullMemory, DumpPath, _pExceptionPtrs);
		}

		if(Desc.PromptAfterDump)
		{
			oMSGBOX_DESC d;
			oStringPath Name;
			oModuleGetName(Name);
			d.Title = Name;
			d.Type = oMSGBOX_ERR;
			oMsgBox(d, "%s\n\nThe program will now exit.%s", _pUserErrorMessage, Mini || Full ? "\n\nA .dmp file has been written." : "\n\n.dmp file was not written.");
		}
		std::exit(-1);
	}

protected:
	oRef<oWinDbgHelp> DbgHelp;
	oRef<threadsafe oStreamWriter> LogFile;
	oStringPath LogPath;
	oStringPath MiniDumpBase;
	oStringPath FullDumpBase;
	oREPORTING_DESC Desc;
	typedef oArray<size_t, 256> array_t;
	array_t FilteredMessages;
	oArray<oReportingVPrint, 8> VPrintStack;
	oRecursiveMutex Mutex;
	bool bDialogBoxesEnabled;
};

// {338D483B-7793-4BE1-90B1-4BB986B3EC2D}
const oGUID oReportingContext::GUID = { 0x338d483b, 0x7793, 0x4be1, { 0x90, 0xb1, 0x4b, 0xb9, 0x86, 0xb3, 0xec, 0x2d } };

oReportingContext::oReportingContext()
	: DbgHelp(oWinDbgHelp::Singleton())
	, bDialogBoxesEnabled(true)
{
	PushReporter(DefaultVPrint);
	std::set_terminate(ReportErrorAndExit);
}

oReportingContext::~oReportingContext()
{
}

void oReportingContext::EnableErrorDialogBoxes(bool _bValue)
{
	bDialogBoxesEnabled = _bValue;
	if (_bValue)
	{
		// Enable asserts and error dialogs
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW );
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW );
	}
	else
	{
		// Disable all crt asserts and error dialogs 
		_CrtSetReportFile(_CRT_ASSERT, stderr);
		_CrtSetReportFile(_CRT_ERROR, stderr);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	}
}

void oReportingContext::SetDesc(const oREPORTING_DESC& _Desc)
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);

	Desc = _Desc;
	if (Desc.LogFilePath)
	{
		oStringPath tmp;
		oCleanPath(tmp.c_str(), tmp.capacity(), Desc.LogFilePath);
		if (oStricmp(LogPath, tmp))
		{
			LogFile = nullptr;
			if (!oStreamLogWriterCreate(tmp, &LogFile))
			{
				LogPath.clear();
				oWARN("Failed to open log file \"%s\"\n%s: %s", tmp.c_str(), oAsString(oErrorGetLast()), oErrorGetLastString());
			}

			oCRTLeakTracker::Singleton()->UntrackAllocation(thread_cast<oStreamWriter*>(LogFile.c_ptr())); // thread cast ok, we're still in initialization
		}

		// make a copy of the path and attach it to the desc for future
		// GetDesc() calls and reassign pointer.
		LogPath = tmp;
		Desc.LogFilePath = LogPath.c_str();
	}

	else 
		LogFile = nullptr;

	if (Desc.MiniDumpBase)
	{
		MiniDumpBase = Desc.MiniDumpBase;
		Desc.MiniDumpBase = MiniDumpBase.c_str();
	}

	if (Desc.FullDumpBase)
	{
		FullDumpBase = Desc.FullDumpBase;
		Desc.FullDumpBase = FullDumpBase.c_str();
	}
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

bool oReportingAreDialogBoxesEnabled()
{
	return oReportingContext::Singleton()->AreDialogBoxesEnabled();
}
void oReportingEnableErrorDialogBoxes(bool _bValue)
{
	oReportingContext::Singleton()->EnableErrorDialogBoxes(_bValue);
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
	cur += oPrintf(format, MESSAGE_PREFIX, _Type == oMSGBOX_WARN ? "Warning" : "Error");
	oStrcpy(cur, std::distance(cur, end), _String);

	char path[_MAX_PATH];
	oSystemGetPath(path, oSYSPATH_APP_FULL);
	char title[1024];
	oPrintf(title, "%s (%s)", DIALOG_BOX_TITLE, path);

	oMSGBOX_DESC mb;
	mb.Type = _Type;
	mb.Title = title;
	return GetAction(oMsgBox(mb, "%s", format));
}

#define oACCUM_PRINTF(_Format, ...) do \
	{	res = oPrintf(_StrDestination + len, _SizeofStrDestination - len - 1, _Format, ## __VA_ARGS__); \
		if (res == -1) goto TRUNCATION; \
		len += res; \
	} while(false)

#define oACCUM_VPRINTF(_Format, _Args) do \
	{	res = oVPrintf(_StrDestination + len, _SizeofStrDestination - len - 1, _Format, _Args); \
		if (res == -1) goto TRUNCATION; \
		len += res; \
	} while(false)

void PrintCallStackToString(char* _StrDestination, size_t _SizeofStrDestination, bool _FilterStdBind)
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
		oPrintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
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

	if (_Desc.PrefixTimestamp)
	{
		oNTPTimestamp now = 0;
		oSystemGetDate(&now);
		res = (int)oDateStrftime(_StrDestination + len, _SizeofStrDestination - len - 1, oDATE_TEXT_SORTABLE_FORMAT_MS " ", now, oDATE_TO_LOCAL);
		if (res == 0) goto TRUNCATION;
		len += res;
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
	oPrintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
	return _StrDestination + _SizeofStrDestination;
}

template<size_t size> inline char* FormatAssertMessage(char (&_StrDestination)[size], const oREPORTING_DESC& _Desc, const oASSERTION& _Assertion, const char* _Format, va_list _Args) { return FormatAssertMessage(_StrDestination, size, _Desc, _Assertion, _Format, _Args); }

oASSERT_ACTION oReportingContext::DefaultVPrint(const oASSERTION& _Assertion, threadsafe oStreamWriter* _pLogFile, const char* _Format, va_list _Args)
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
		oSTREAM_WRITE w;
		w.pData = msg;
		w.Range = oSTREAM_RANGE(oSTREAM_APPEND, strlen(msg));
		_pLogFile->Write(w);
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
			if (oReportingAreDialogBoxesEnabled())
			{
				if (desc.PromptAsserts)
					action = ShowMsgBox(_Assertion, oMSGBOX_DEBUG, msg);
			}
			else
			{
				oTRACE("%s (%i): %s", _Assertion.Filename, _Assertion.Line, msg);
				abort();
			}

			#ifndef _DEBUG
				oReportingContext::Singleton()->DumpAndTerminate(nullptr, nullptr);
			#endif

			break;

		default: oASSERT_NOEXECUTION;
	}

	return action;
}


class ReportContextExceptionHandler : public oProcessSingleton<ReportContextExceptionHandler>
{
	// @oooii-tony: This is truly annoying. I would really like to merge this 
	// singleton into oReportingContext, but for whatever reason, processes spawned
	// from other processes crash when that is the case. Looking at the singleton
	// trace stack, it should collapse nicely because this gets instantiated 
	// after oReportingContext. So why can't this be simplified in code, unified
	// in concept, and be installed earlier to catch even more problems?

public:
	static const oGUID GUID;
	oStd::mutex Mutex;

	ReportContextExceptionHandler()
	{
		AddVectoredExceptionHandler(0, &ReportContextExceptionHandler::HandleAccessViolation);
	}

	~ReportContextExceptionHandler()
	{
		RemoveVectoredExceptionHandler(&ReportContextExceptionHandler::HandleAccessViolation);
	}

	// Allows us to break execution when an access violation occurs
	static LONG CALLBACK HandleAccessViolation(PEXCEPTION_POINTERS _pExceptionPointers)
	{
		ReportContextExceptionHandler* p = ReportContextExceptionHandler::Singleton();

		auto DumpAndPrint = [&](const char* _ErrorMessage)
		{
			if (p->Mutex.try_lock())
			{
#ifdef _DEBUG
				oASSERT(false, "%s", _ErrorMessage);
				p->Mutex.unlock(); // No need to unlock when DumpAndTerminate is called as the app will exit
#else
				oReportingContext::Singleton()->DumpAndTerminate(_pExceptionPointers, _ErrorMessage);
#endif
			}
		};

		EXCEPTION_RECORD* pRecord = _pExceptionPointers->ExceptionRecord;
		switch (pRecord->ExceptionCode)
		{
		case EXCEPTION_STACK_OVERFLOW:
			DumpAndPrint("Stack overflow");
			break;

		case EXCEPTION_ACCESS_VIOLATION:
			{
				void* pAddress = (void*)pRecord->ExceptionInformation[1];
				const char* err = (0 == pRecord->ExceptionInformation[0]) ? "Read" : "Write";
				oStringL ErrorMessage;
				sprintf_s(ErrorMessage.c_str(), "%s access violation at 0x%p", err, pAddress);

				oDebuggerAllocationInfo AllocationInfo;
				if(	oDebuggerGuardedInfo(pAddress, &AllocationInfo) )
				{
					if(oInvalid == AllocationInfo.ThreadFreedOn)
					{
						oStrAppendf(ErrorMessage, "Guarded allocation attempting to access outside of allocation");
					}
					else
					{
						double TimePassed = oTimer() - AllocationInfo.FreedTimer;
						oStrAppendf(ErrorMessage, "Freed on thread %d %f seconds ago", AllocationInfo.ThreadFreedOn, TimePassed);
					}
					
				}
				DumpAndPrint(ErrorMessage);
			}

		default:
			break;
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}
};

void PureVirtualCallHandler(void)
{
	int* generate_access_violation = nullptr;
	generate_access_violation[0] = 0;
}

// {9840E986-9ADE-4D11-AFCE-AB2D8AC530C0}
const oGUID ReportContextExceptionHandler::GUID = { 0x9840e986, 0x9ade, 0x4d11, { 0xaf, 0xce, 0xab, 0x2d, 0x8a, 0xc5, 0x30, 0xc0 } };

struct ReportContextExceptionHandlerInstaller
{
	ReportContextExceptionHandlerInstaller()
	{
		oProcessHeapEnsureRunning(); // ensure the process heap is instantiated before the Singleton below so it is tracked
		ReportContextExceptionHandler::Singleton();
		_set_purecall_handler(PureVirtualCallHandler);
	}
};

// @oooii-kevin: OK Static, we need to make sure the exception handler is installed very early 
static ReportContextExceptionHandlerInstaller GInstallReportContextExceptionHandler;

