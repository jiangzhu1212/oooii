// $(header)
#include <oPlatform/oReporting.h>
#include <oBasis/oAlgorithm.h>
#include <oBasis/oArray.h>
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oSystem.h>
#include "oDbgHelp.h"

template<> const char* oAsString(const oASSERT_TYPE& _Type)
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
	~oReportingContext();

	void SetDesc(const oREPORTING_DESC& _Desc);
	inline void GetDesc(oREPORTING_DESC* _pDesc) { *_pDesc = Desc; }
	bool PushReporter(oReportingVPrint _Reporter);
	oReportingVPrint PopReporter();
	inline void AddFilter(size_t _AssertionID) { oPushBackUnique(FilteredMessages, _AssertionID); }
	inline void RemoveFilter(size_t _AssertionID) { oFindAndErase(FilteredMessages, _AssertionID); }
	oASSERT_ACTION VPrint(const oASSERTION& _Assertion, const char* _Format, va_list _Args);
	static oASSERT_ACTION DefaultVPrint(const oASSERTION& _Assertion, void* _hLogFile, const char* _Format, va_list _Args);
	static const oGUID GUID;

protected:
	oRef<oDbgHelp> DbgHelp;
	oHFILE hLogFile;
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
	, hLogFile(nullptr)
{
	PushReporter(DefaultVPrint);
}

oReportingContext::~oReportingContext()
{
	if (hLogFile)
		oFileClose(hLogFile);
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
			if (hLogFile)
			{
				oFileClose(hLogFile);
				hLogFile = nullptr;
			}

			if (!oFileOpen(tmp, oFILE_OPEN_TEXT_WRITE, &hLogFile))
			{
				// ensure the dir exists before giving up
				oStringPath tmpdir = tmp;
				*oGetFilebase(tmpdir.c_str()) = 0;
				if (oFileCreateFolder(tmpdir) || oErrorGetLast() == oERROR_REDUNDANT)
				{
					if (!oFileOpen(tmp, oFILE_OPEN_TEXT_WRITE, &hLogFile))
					{
						LogPath.clear();
						oWARN("Failed to open log file \"%s\"", tmp.c_str());
					}
				}

				else
					oWARN("Failed to create path for log file \"%s\"", tmp.c_str());
			}
		}

		// make a copy of the path and attach it to the desc for future
		// GetDesc() calls and reassign pointer.
		LogPath = tmp;
		Desc.LogFilePath = LogPath.c_str();
	}

	else if (hLogFile)
	{
		oFileClose(hLogFile);
		hLogFile = 0;
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
		return VPrintMessage(_Assertion, hLogFile, _Format, _Args);
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

oASSERT_ACTION oReportingContext::DefaultVPrint(const oASSERTION& _Assertion, void* _hLogFile, const char* _Format, va_list _Args)
{
	oHFILE hLogFile = (oHFILE)_hLogFile;

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

	if (hLogFile)
		oFileWrite(hLogFile, msg, strlen(msg), true);

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
