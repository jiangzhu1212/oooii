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
#include <oPlatform/oReporting.h>
#include <oStd/algorithm.h>
#include <oStd/fixed_vector.h>
#include <oBasis/oError.h>
#include <oStd/fixed_string.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oSystem.h>
#include <oPlatform/oStandards.h>
#include <oPlatform/Windows/oWinAsString.h>
#include "SoftLink/oWinDbgHelp.h"
#include "oCRTLeakTracker.h"
#include "oFileInternal.h"

#define oEXCEPTION_PURE_VIRTUAL_CALL 0x8badc0de
#define oEXCEPTION_BAD_EXCEPTION 0x8badec10

namespace oStd {

const char* as_string(const oStd::assert_type::value& _Type)
{
	switch (_Type)
	{
		case oStd::assert_type::trace: return "Trace";
		case oStd::assert_type::assertion: return "Error";
		oNODEFAULT;
	}
}

} // namespace oStd

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
		return oErrorSetLast(std::errc::invalid_argument, "Failed to open %s for write.", oSAFESTRN(_Path));

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
		return oErrorSetLast(std::errc::io_error, "Failed to write dump file %s", oSAFESTRN(_Path));
	return success;
}

//static bool oWinWriteDumpFile(EXCEPTION_POINTERS* _pExceptionPointers)
//{
//	oStd::path_string DumpPath;
//	oSystemGetPath(DumpPath, oSYSPATH_APP_FULL);
//	oTrimFileExtension(DumpPath);
//	oStrAppendf(DumpPath, ".dmp");
//	return oWinWriteDumpFile(DumpPath, _pExceptionPointers);
//}

void ReportErrorAndExit()
{
	oTRACEA("std::terminate called");
	oASSERT(false, "std::terminate called");
}

struct oReportingContext : oProcessSingleton<oReportingContext>
{
	oReportingContext();
	~oReportingContext();

	void SetDesc(const oREPORTING_DESC& _Desc);
	inline void GetDesc(oREPORTING_DESC* _pDesc) { *_pDesc = Desc; }
	bool PushReporter(oReportingVPrint _Reporter);
	oReportingVPrint PopReporter();
	inline void AddFilter(size_t _AssertionID) { oStd::push_back_unique(FilteredMessages, _AssertionID); }
	inline void RemoveFilter(size_t _AssertionID) { oStd::find_and_erase(FilteredMessages, _AssertionID); }
	oStd::assert_action::value VPrint(const oStd::assert_context& _Assertion, const char* _Format, va_list _Args);
	static oStd::assert_action::value DefaultVPrint(const oStd::assert_context& _Assertion, threadsafe oStreamWriter* _pLogFile, const char* _Format, va_list _Args);
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
		oStd::sstring DumpStamp = VersionString;

		oStd::ntp_date now;
		oSystemGetDate(&now);
		oStd::sstring StrNow;
		oStd::strftime(DumpStamp.c_str() + DumpStamp.length(), DumpStamp.capacity() - DumpStamp.length(), oStd::syslog_local_date_format, now, oStd::date_conversion::to_local);
		oStd::replace(StrNow, DumpStamp, ":", "_");
		oStd::replace(DumpStamp, StrNow, ".", "_");

		bool Mini = false;
		bool Full = false;

		oStd::path_string DumpPath;
		if (!Desc.MiniDumpBase.empty())
		{
			oPrintf(DumpPath, "%s%s.dmp", Desc.MiniDumpBase, DumpStamp.c_str());
			Mini = oWinWriteDumpFile(MiniDumpNormal, DumpPath, _pExceptionPtrs);
		}

		if (!Desc.FullDumpBase.empty())
		{
			oPrintf(DumpPath, "%s%s.dmp", Desc.FullDumpBase, DumpStamp.c_str());
			Full = oWinWriteDumpFile(MiniDumpWithFullMemory, DumpPath, _pExceptionPtrs);
		}

		if(!Desc.PostDumpExecution.empty())
		{
			// Use raw system command to avoid code complexity during dump
			system(Desc.PostDumpExecution.c_str());
		}

		if(Desc.PromptAfterDump)
		{
			oMSGBOX_DESC d;
			oStd::path_string Name;
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
	oStd::sstring VersionString;
	oREPORTING_DESC Desc;
	typedef oStd::fixed_vector<size_t, 256> array_t;
	array_t FilteredMessages;
	oStd::fixed_vector<oReportingVPrint, 8> VPrintStack;
	oConcurrency::recursive_mutex Mutex;
	bool bDialogBoxesEnabled;
};

// From oWindows.h
void oWinDumpAndTerminate(EXCEPTION_POINTERS* _pExceptionPtrs, const char* _pUserErrorMessage)
{
	return oReportingContext::Singleton()->DumpAndTerminate(_pExceptionPtrs, _pUserErrorMessage);
}

// {338D483B-7793-4BE1-90B1-4BB986B3EC2D}
const oGUID oReportingContext::GUID = { 0x338d483b, 0x7793, 0x4be1, { 0x90, 0xb1, 0x4b, 0xb9, 0x86, 0xb3, 0xec, 0x2d } };
oSINGLETON_REGISTER(oReportingContext);

oReportingContext::oReportingContext()
	: DbgHelp(oWinDbgHelp::Singleton())
	, bDialogBoxesEnabled(true)
{
	PushReporter(DefaultVPrint);
	std::set_terminate(ReportErrorAndExit);

	// Cache the version string now in case we have to dump later (so we don't call complicated module code)
	{
		oMODULE_DESC ModuleDesc;
		oModuleGetDesc(&ModuleDesc);

		VersionString[0] = 'V';
		oStd::to_string(&VersionString[1], VersionString.capacity() - 1, ModuleDesc.ProductVersion);
		oStrAppendf(VersionString, "D");
	}
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
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);

	oStd::path OldLogPath(Desc.LogFilePath);
	Desc = _Desc;
	if (Desc.LogFilePath)
	{
		Desc.LogFilePath = _Desc.LogFilePath;
		if (oStricmp(OldLogPath, Desc.LogFilePath))
		{
			LogFile = nullptr;
			if (!oStreamLogWriterCreate(Desc.LogFilePath, &LogFile))
			{
				oTRACE("WARNING: Failed to open log file \"%s\"\n%s: %s", Desc.LogFilePath.c_str(), oErrorAsString(oErrorGetLast()), oErrorGetLastString());
				Desc.LogFilePath.clear();
			}
			oCRTLeakTracker::Singleton()->UntrackAllocation(thread_cast<oStreamWriter*>(LogFile.c_ptr())); // thread cast ok, we're still in initialization
		}
	}
	else
		LogFile = nullptr;
}

bool oReportingContext::PushReporter(oReportingVPrint _Reporter)
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);

	if (VPrintStack.size() >= VPrintStack.capacity())
		return false;
	VPrintStack.push_back(_Reporter);
	return true;
}

oReportingVPrint oReportingContext::PopReporter()
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);

	oReportingVPrint fn = nullptr;
	if (!VPrintStack.empty())
	{
		fn = VPrintStack.back();
		VPrintStack.pop_back();
	}
	return fn;
}

oStd::assert_action::value oReportingContext::VPrint(const oStd::assert_context& _Assertion, const char* _Format, va_list _Args)
{
	size_t ID = oStd::fnv1a<size_t>(_Format);
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);

	if (!oStd::contains(FilteredMessages, ID) && !VPrintStack.empty())
	{
		oReportingVPrint VPrintMessage = VPrintStack.back();
		return VPrintMessage(_Assertion, LogFile, _Format, _Args);
	}

	return oStd::assert_action::ignore;
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

oStd::assert_action::value oStd::vtracef(const oStd::assert_context& _Assertion, const char* _Format, va_list _Args)
{
	return oReportingContext::Singleton()->VPrint(_Assertion, _Format, _Args);
}

static oStd::assert_action::value GetAction(oMSGBOX_RESULT _Result)
{
	switch (_Result)
	{
		case oMSGBOX_ABORT: return oStd::assert_action::abort;
		case oMSGBOX_BREAK: return oStd::assert_action::debug;
		case oMSGBOX_IGNORE: return oStd::assert_action::ignore_always;
		default: break;
	}

	return oStd::assert_action::ignore;
}

static oStd::assert_action::value ShowMsgBox(const oStd::assert_context& _Assertion, oMSGBOX_TYPE _Type, const char* _String)
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
	
	if (oSTRVALID(_Assertion.Expression))
		cur += oPrintf(cur, std::distance(cur, end), "%s\n", _Assertion.Expression);

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
		size_t TLMLength = oStrlen(kStackTooLargeMessage);
		oPrintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
}

char* FormatAssertMessage(char* _StrDestination, size_t _SizeofStrDestination, const oREPORTING_DESC& _Desc, const oStd::assert_context& _Assertion, const char* _Format, va_list _Args)
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
		oStd::ntp_timestamp now = 0;
		oSystemGetDate(&now);
		res = (int)oStd::strftime(_StrDestination + len, _SizeofStrDestination - len - 1, oStd::sortable_date_ms_format, now, oStd::date_conversion::to_local);
		oACCUM_PRINTF(" ");
		if (res == 0) goto TRUNCATION;
		len += res;
	}

	if (_Desc.PrefixMsgType)
		oACCUM_PRINTF("%s ", oStd::as_string(_Assertion.Type));

	if (_Desc.PrefixThreadId)
	{
		oStd::mstring exec;
		oACCUM_PRINTF("%s ", oSystemGetExecutionPath(exec));
	}

	if (_Desc.PrefixMsgId)
		oACCUM_PRINTF("{0x%08x} ", oStd::fnv1a<unsigned int>(_Format));

	oACCUM_VPRINTF(_Format, _Args);
	return _StrDestination + len;

TRUNCATION:
	static const char* kStackTooLargeMessage = "\n... truncated ...";
	size_t TLMLength = oStrlen(kStackTooLargeMessage);
	oPrintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
	return _StrDestination + _SizeofStrDestination;
}

template<size_t size> inline char* FormatAssertMessage(char (&_StrDestination)[size], const oREPORTING_DESC& _Desc, const oStd::assert_context& _Assertion, const char* _Format, va_list _Args) { return FormatAssertMessage(_StrDestination, size, _Desc, _Assertion, _Format, _Args); }

oStd::assert_action::value oReportingContext::DefaultVPrint(const oStd::assert_context& _Assertion, threadsafe oStreamWriter* _pLogFile, const char* _Format, va_list _Args)
{
	oREPORTING_DESC desc;
	oReportingGetDesc(&desc);

	bool addCallStack = desc.PrintCallstack && (_Assertion.Type == oStd::assert_type::assertion);

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
		w.Range = oSTREAM_RANGE(oSTREAM_APPEND, oStrlen(msg));
		_pLogFile->Write(w);
	}

	// Output message
	oStd::assert_action::value action = _Assertion.DefaultResponse;
	switch (_Assertion.Type)
	{
		case oStd::assert_type::trace:
			break;

		case oStd::assert_type::assertion:
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

		oNODEFAULT;
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
		_set_purecall_handler(ReportContextExceptionHandler::PureVirtualCallHandler);
		set_unexpected(ReportContextExceptionHandler::UnexpectedHandler);
	}

	~ReportContextExceptionHandler()
	{
		RemoveVectoredExceptionHandler(&ReportContextExceptionHandler::HandleAccessViolation);
	}

	void DumpAndTerminate(EXCEPTION_POINTERS* _pExceptionPointers, const char* _ErrorMessage = "Unhandled Exception")
	{
		if (Mutex.try_lock())
		{
			#ifdef _DEBUG
				oASSERT_TRACE(oStd::assert_type::assertion, oStd::assert_action::abort, "", "%s", _ErrorMessage);
				oASSERT(false, "%s", _ErrorMessage);
				Mutex.unlock(); // No need to unlock when DumpAndTerminate is called as the app will exit
			#else
				oReportingContext::Singleton()->DumpAndTerminate(_pExceptionPointers, _ErrorMessage);
			#endif
		}
	};

	static void PureVirtualCallHandler()
	{
		RaiseException(oEXCEPTION_PURE_VIRTUAL_CALL, EXCEPTION_NONCONTINUABLE, 0, nullptr);
	}
	static void UnexpectedHandler()
	{
		RaiseException(oEXCEPTION_BAD_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, nullptr);
	}

	// Allows us to break execution when an access violation occurs
	static LONG CALLBACK HandleAccessViolation(PEXCEPTION_POINTERS _pExceptionPointers)
	{
    const unsigned int VISUAL_STUDIO_CPP_EXCEPTION = 0xe06d7363;

		ReportContextExceptionHandler* p = ReportContextExceptionHandler::Singleton();

		EXCEPTION_RECORD* pRecord = _pExceptionPointers->ExceptionRecord;
		switch (pRecord->ExceptionCode)
		{
			case VISUAL_STUDIO_CPP_EXCEPTION:
				break;

			case oEXCEPTION_PURE_VIRTUAL_CALL:
			{
				p->DumpAndTerminate(_pExceptionPointers, "pure virtual function call");
				break;
			}
			
			case oEXCEPTION_BAD_EXCEPTION:
			{
				p->DumpAndTerminate(_pExceptionPointers, "bad exception");
				break;
			}

			case EXCEPTION_ACCESS_VIOLATION:
			{
				void* pAddress = (void*)pRecord->ExceptionInformation[1];
				const char* err = (0 == pRecord->ExceptionInformation[0]) ? "Read" : "Write";
				oStd::lstring ErrorMessage;
				sprintf_s(ErrorMessage.c_str(), "%s access violation at 0x%p", err, pAddress);

				oDebuggerAllocationInfo AllocationInfo;
				if (oDebuggerGuardedInfo(pAddress, &AllocationInfo))
				{
					if (oInvalid == AllocationInfo.ThreadFreedOn)
						oStrAppendf(ErrorMessage, "Guarded allocation attempting to access outside of allocation");
					else
					{
						double TimePassed = oTimer() - AllocationInfo.FreedTimer;
						oStrAppendf(ErrorMessage, "Freed on thread %d %f seconds ago", AllocationInfo.ThreadFreedOn, TimePassed);
					}
				}

				p->DumpAndTerminate(_pExceptionPointers, ErrorMessage);
				break;
			}
			// Ensure any exception that is to be handled is listed explicitly. There
			// are some working-as-intended uses of exceptions by Windows that should 
			// not terminate execution, but instead should be passed through.
			case EXCEPTION_DATATYPE_MISALIGNMENT:
			case EXCEPTION_BREAKPOINT:
			case EXCEPTION_SINGLE_STEP:
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			case EXCEPTION_FLT_DENORMAL_OPERAND:
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			case EXCEPTION_FLT_INEXACT_RESULT:
			case EXCEPTION_FLT_INVALID_OPERATION:
			case EXCEPTION_FLT_OVERFLOW:
			case EXCEPTION_FLT_STACK_CHECK:
			case EXCEPTION_FLT_UNDERFLOW:
			case EXCEPTION_INT_DIVIDE_BY_ZERO:
			case EXCEPTION_INT_OVERFLOW:
			case EXCEPTION_PRIV_INSTRUCTION:
			case EXCEPTION_IN_PAGE_ERROR:
			case EXCEPTION_ILLEGAL_INSTRUCTION:
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			case EXCEPTION_STACK_OVERFLOW:
			case EXCEPTION_INVALID_DISPOSITION:
			case EXCEPTION_GUARD_PAGE:
			case EXCEPTION_INVALID_HANDLE:
					p->DumpAndTerminate(_pExceptionPointers, oWinAsStringExceptionCode(pRecord->ExceptionCode));
				break;

			default:
				break;
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}
};

// {9840E986-9ADE-4D11-AFCE-AB2D8AC530C0}
const oGUID ReportContextExceptionHandler::GUID = { 0x9840e986, 0x9ade, 0x4d11, { 0xaf, 0xce, 0xab, 0x2d, 0x8a, 0xc5, 0x30, 0xc0 } };
oSINGLETON_REGISTER(ReportContextExceptionHandler);

struct ReportContextExceptionHandlerInstaller
{
	ReportContextExceptionHandlerInstaller()
	{
		oProcessHeapEnsureRunning(); // ensure the process heap is instantiated before the Singleton below so it is tracked
		ReportContextExceptionHandler::Singleton();
	}
};

// @oooii-kevin: OK Static, we need to make sure the exception handler is installed very early 
static ReportContextExceptionHandlerInstaller GInstallReportContextExceptionHandler;

