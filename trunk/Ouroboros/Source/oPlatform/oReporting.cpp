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
#include <oBase/algorithm.h>
#include <oBase/fixed_string.h>
#include <oBase/fixed_vector.h>
#include <oCore/debugger.h>
#include <oBasis/oError.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oStandards.h>
#include <oPlatform/oStream.h>
#include <oPlatform/Windows/oWinAsString.h>
#include "oCRTLeakTracker.h"
#include "oWinExceptionHandler.h"

using namespace ouro;

namespace ouro {

const char* as_string(const assert_type::value& _Type)
{
	switch (_Type)
	{
		case assert_type::trace: return "Trace";
		case assert_type::assertion: return "Error";
		oNODEFAULT;
	}
}

} // namespace ouro

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
	inline void AddFilter(size_t _AssertionID) { push_back_unique(FilteredMessages, _AssertionID); }
	inline void RemoveFilter(size_t _AssertionID) { find_and_erase(FilteredMessages, _AssertionID); }
	assert_action::value VPrint(const assert_context& _Assertion, const char* _Format, va_list _Args);
	static assert_action::value DefaultVPrint(const assert_context& _Assertion, threadsafe oStreamWriter* _pLogFile, const char* _Format, va_list _Args);
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
		sstring DumpStamp = VersionString;

		ouro::ntp_date now;
		ouro::system::now(&now);
		sstring StrNow;
		strftime(DumpStamp.c_str() + DumpStamp.length(), DumpStamp.capacity() - DumpStamp.length(), ouro::syslog_local_date_format, now, ouro::date_conversion::to_local);
		replace(StrNow, DumpStamp, ":", "_");
		replace(DumpStamp, StrNow, ".", "_");

		bool Mini = false;
		bool Full = false;

		path_string DumpPath;
		if (!Desc.MiniDumpBase.empty())
		{
			snprintf(DumpPath, "%s%s.dmp", Desc.MiniDumpBase, DumpStamp.c_str());
			Mini = ouro::debugger::dump(path(DumpPath), false, _pExceptionPtrs);
		}

		if (!Desc.FullDumpBase.empty())
		{
			snprintf(DumpPath, "%s%s.dmp", Desc.FullDumpBase, DumpStamp.c_str());
			Full = ouro::debugger::dump(path(DumpPath), true, _pExceptionPtrs);
		}

		if(!Desc.PostDumpExecution.empty())
		{
			// Use raw system command to avoid code complexity during dump
			::system(Desc.PostDumpExecution.c_str());
		}

		if(Desc.PromptAfterDump)
		{
			oMSGBOX_DESC d;
			path Name = ouro::this_module::path();
			d.Title = Name;
			d.Type = oMSGBOX_ERR;
			oMsgBox(d, "%s\n\nThe program will now exit.%s", _pUserErrorMessage, Mini || Full ? "\n\nA .dmp file has been written." : "\n\n.dmp file was not written.");
		}
		std::exit(-1);
	}

protected:
	intrusive_ptr<threadsafe oStreamWriter> LogFile;
	sstring VersionString;
	oREPORTING_DESC Desc;
	typedef fixed_vector<size_t, 256> array_t;
	array_t FilteredMessages;
	fixed_vector<oReportingVPrint, 8> VPrintStack;
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
	: bDialogBoxesEnabled(true)
{
	PushReporter(DefaultVPrint);
	std::set_terminate(ReportErrorAndExit);

	// Cache the version string now in case we have to dump later (so we don't call complicated module code)
	{
		ouro::module::info mi = ouro::this_module::get_info();
		VersionString[0] = 'V';
		to_string(&VersionString[1], VersionString.capacity() - 1, mi.version);
		sncatf(VersionString, "D");
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

	path OldLogPath(Desc.LogFilePath);
	Desc = _Desc;
	if (Desc.LogFilePath)
	{
		Desc.LogFilePath = _Desc.LogFilePath;
		if (_stricmp(OldLogPath, Desc.LogFilePath))
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

assert_action::value oReportingContext::VPrint(const assert_context& _Assertion, const char* _Format, va_list _Args)
{
	size_t ID = fnv1a<size_t>(_Format);
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);

	if (!contains(FilteredMessages, ID) && !VPrintStack.empty())
	{
		oReportingVPrint VPrintMessage = VPrintStack.back();
		return VPrintMessage(_Assertion, LogFile, _Format, _Args);
	}

	return assert_action::ignore;
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

namespace ouro {
	assert_action::value vtracef(const assert_context& _Assertion, const char* _Format, va_list _Args)
	{
		return oReportingContext::Singleton()->VPrint(_Assertion, _Format, _Args);
	}
} // namespace ouro

static assert_action::value GetAction(oMSGBOX_RESULT _Result)
{
	switch (_Result)
	{
		case oMSGBOX_ABORT: return assert_action::abort;
		case oMSGBOX_BREAK: return assert_action::debug;
		case oMSGBOX_IGNORE: return assert_action::ignore_always;
		default: break;
	}

	return assert_action::ignore;
}

static assert_action::value ShowMsgBox(const assert_context& _Assertion, oMSGBOX_TYPE _Type, const char* _String)
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
	cur += snprintf(format, MESSAGE_PREFIX, _Type == oMSGBOX_WARN ? "Warning" : "Error");
	
	if (oSTRVALID(_Assertion.Expression))
		cur += snprintf(cur, std::distance(cur, end), "%s\n", _Assertion.Expression);

	strlcpy(cur, _String, std::distance(cur, end));

	path AppPath = ouro::filesystem::app_path(true);
	char title[1024];
	snprintf(title, "%s (%s)", DIALOG_BOX_TITLE, AppPath.c_str());

	oMSGBOX_DESC mb;
	mb.Type = _Type;
	mb.Title = title;
	return GetAction(oMsgBox(mb, "%s", format));
}

#define oACCUM_PRINTF(_Format, ...) do \
	{	res = snprintf(_StrDestination + len, _SizeofStrDestination - len - 1, _Format, ## __VA_ARGS__); \
		if (res == -1) goto TRUNCATION; \
		len += res; \
	} while(false)

#define oACCUM_VPRINTF(_Format, _Args) do \
	{	res = ouro::vsnprintf(_StrDestination + len, _SizeofStrDestination - len - 1, _Format, _Args); \
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
	ouro::debugger::symbol address = 0;
	bool IsStdBind = false;
	while (ouro::debugger::callstack(&address, 1, offset++))
	{
		if (nSymbols++ == 0) // if we have a callstack, label it
		{
			res = _snprintf_s(_StrDestination, _SizeofStrDestination, _TRUNCATE, "\nCall Stack:\n");
			if (res == -1) goto TRUNCATION;
			len += res;
		}

		bool WasStdBind = IsStdBind;
		res = ouro::debugger::format(&_StrDestination[len], _SizeofStrDestination - len - 1, address, "", &IsStdBind);
		if (res == -1) goto TRUNCATION;
		len += res;

		if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
			offset += 5;
	}

	return;

	TRUNCATION:
		static const char* kStackTooLargeMessage = "\n... truncated ...";
		size_t TLMLength = strlen(kStackTooLargeMessage);
		snprintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
}

char* FormatAssertMessage(char* _StrDestination, size_t _SizeofStrDestination, const oREPORTING_DESC& _Desc, const assert_context& _Assertion, const char* _Format, va_list _Args)
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
		ouro::ntp_timestamp now = 0;
		system::now(&now);
		res = (int)ouro::strftime(_StrDestination + len, _SizeofStrDestination - len - 1, ouro::sortable_date_ms_format, now, ouro::date_conversion::to_local);
		oACCUM_PRINTF(" ");
		if (res == 0) goto TRUNCATION;
		len += res;
	}

	if (_Desc.PrefixMsgType)
		oACCUM_PRINTF("%s ", as_string(_Assertion.Type));

	if (_Desc.PrefixThreadId)
	{
		mstring exec;
		oACCUM_PRINTF("%s ", ouro::system::exec_path(exec));
	}

	if (_Desc.PrefixMsgId)
		oACCUM_PRINTF("{0x%08x} ", fnv1a<unsigned int>(_Format));

	oACCUM_VPRINTF(_Format, _Args);
	return _StrDestination + len;

TRUNCATION:
	static const char* kStackTooLargeMessage = "\n... truncated ...";
	size_t TLMLength = strlen(kStackTooLargeMessage);
	snprintf(_StrDestination + _SizeofStrDestination - 1 - TLMLength, TLMLength + 1, kStackTooLargeMessage);
	return _StrDestination + _SizeofStrDestination;
}

template<size_t size> inline char* FormatAssertMessage(char (&_StrDestination)[size], const oREPORTING_DESC& _Desc, const assert_context& _Assertion, const char* _Format, va_list _Args) { return FormatAssertMessage(_StrDestination, size, _Desc, _Assertion, _Format, _Args); }

assert_action::value oReportingContext::DefaultVPrint(const assert_context& _Assertion, threadsafe oStreamWriter* _pLogFile, const char* _Format, va_list _Args)
{
	oREPORTING_DESC desc;
	oReportingGetDesc(&desc);

	bool addCallStack = desc.PrintCallstack && (_Assertion.Type == assert_type::assertion);

	// add prefixes to original message
	char msg[oKB(8)];
	char* cur = FormatAssertMessage(msg, desc, _Assertion, _Format, _Args);
	char* end = msg + sizeof(msg);

	if (addCallStack)
		PrintCallStackToString(cur, std::distance(cur, end), true);

	// Always print any message to the debugger output
	ouro::debugger::print(msg);

	// And to log file
	if (_pLogFile)
	{
		oSTREAM_WRITE w;
		w.pData = msg;
		w.Range = oSTREAM_RANGE(oSTREAM_APPEND, strlen(msg));
		_pLogFile->Write(w);
	}

	// Output message
	assert_action::value action = _Assertion.DefaultResponse;
	switch (_Assertion.Type)
	{
		case assert_type::trace:
			break;

		case assert_type::assertion:
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

static void DumpAndTerminate(const char* _ErrorMessage, const oWinCppException& _CppException, uintptr_t _ExceptionContext)
{
	EXCEPTION_POINTERS* pExceptionPointers = (EXCEPTION_POINTERS*) _ExceptionContext;

	//if (Mutex.try_lock())
	{
		if (!ouro::this_process::has_debugger_attached())
		{
			#ifdef _DEBUG
				oASSERT_TRACE(assert_type::assertion, assert_action::abort, "", "%s", _ErrorMessage);
				oASSERT(false, "%s", _ErrorMessage);
				//Mutex.unlock(); // No need to unlock when DumpAndTerminate is called as the app will exit
			#else
				oReportingContext::Singleton()->DumpAndTerminate(pExceptionPointers, _ErrorMessage);
			#endif
		}
	}
};

#if 0
static void ReportException(const char* _ErrorMessage, const oWinCppException& _CppException, uintptr_t _ExceptionContext)
{
	const char* eType = "";
	if (_pStdException)
	{
		eType = type_name(typeid(*_pStdException).name());
		const char* n = rstrstr(eType, "::");
		if (n)
			eType = n + 2;
	}

	if (oProcessHasDebuggerAttached())
	{
		if (_pStdException)
			oASSERT_TRACE(assert_type::assertion, assert_action::abort, "", "%s: %s", eType, _pStdException->what());
		else
			oASSERT_TRACE(assert_type::assertion, assert_action::abort, "", "%s", _ErrorMessage);
	}
	else
	{
		EXCEPTION_POINTERS* pExceptionPointers = (EXCEPTION_POINTERS*)_ExceptionContext;
		lstring msg(_ErrorMessage);
		if (_pStdException)
			snprintf(msg, "unhandled %s exception\n\n%s", eType, _pStdException->what());
		oReportingContext::Singleton()->DumpAndTerminate(pExceptionPointers, msg);
	}
}
#endif

struct InstallExceptionHandler
{
	InstallExceptionHandler()
	{
		oProcessHeapEnsureRunning(); // ensure the process heap is instantiated before the Singleton below so it is tracked
		oSINGLETON_REGISTER(oWinExceptionHandler);
		oWinExceptionHandler::Singleton()->SetHandler(DumpAndTerminate);
	}
};

static InstallExceptionHandler GInstalledExceptionHandler; // ok static, used to register a singleton
